"""A small private speech to text service.

Only the API in ai/backend talks to it, over the internal network. Audio stays
in memory for the length of one request and is never written or logged, and
transcripts are never logged. Answers carry only text and generic error codes.
"""

import collections
import hmac
import io
import json
import logging
import os
import signal
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs, urlsplit

import av
import numpy as np

import engines

RATE = 16000
CODECS = {"opus", "vorbis", "aac", "mp3", "mp3float", "flac", "pcm_u8", "pcm_s16le", "pcm_s24le", "pcm_s32le", "pcm_f32le"}
LANGUAGES = ("fa", "en")

log = logging.getLogger("stt")


class Rejected(Exception):
    def __init__(self, status, code):
        super().__init__(code)
        self.status = status
        self.code = code


def env_int(name, default):
    return int(os.environ.get(name, "") or default)


class Config:
    def __init__(self):
        self.host = os.environ.get("STT_HOST", "0.0.0.0")
        self.port = env_int("STT_PORT", 8090)
        self.secret = os.environ.get("STT_SECRET", "")
        self.max_seconds = env_int("STT_MAX_SECONDS", 120)
        self.max_bytes = env_int("STT_MAX_BYTES", 4194304)
        self.concurrency = max(1, env_int("STT_CONCURRENCY", 1))
        self.queue = max(0, env_int("STT_QUEUE", 2))
        self.queue_wait = env_int("STT_QUEUE_WAIT_SECONDS", 20)
        self.timeout = env_int("STT_TIMEOUT_SECONDS", 80)
        self.max_loaded = max(1, env_int("STT_MAX_LOADED", 2))
        self.preload = os.environ.get("STT_PRELOAD", "").strip()
        default_engine = os.environ.get("STT_ENGINE", "engines:SegmentEngine")
        self.models = {}
        for lang in LANGUAGES:
            upper = lang.upper()
            path = os.environ.get(f"STT_MODEL_{upper}", "").strip()
            if path and (not path.startswith("/") or os.path.exists(path)):
                engine = os.environ.get(f"STT_ENGINE_{upper}", "") or default_engine
                self.models[lang] = (engine, path)


def container_of(body):
    if len(body) < 12:
        return None
    if body[:4] == b"\x1a\x45\xdf\xa3":
        return "matroska"
    if body[:4] == b"OggS":
        return "ogg"
    if body[4:8] == b"ftyp":
        return "mov"
    if body[:4] == b"fLaC":
        return "flac"
    if body[:4] == b"RIFF" and body[8:12] == b"WAVE":
        return "wav"
    if body[:3] == b"ID3" or mp3_frame(body):
        return "mp3"
    return None


def mp3_frame(body):
    """True for an MPEG audio frame header, which is what a bare mp3 starts with."""
    if body[0] != 0xFF or body[1] & 0xE0 != 0xE0:
        return False
    version = (body[1] >> 3) & 3
    layer = (body[1] >> 1) & 3
    bitrate = body[2] >> 4
    rate = (body[2] >> 2) & 3
    return version != 1 and layer != 0 and bitrate not in (0, 15) and rate != 3


def decode(body, container, max_seconds):
    """Decode to 16 kHz mono float32, giving up as soon as the clip is too long.

    The demuxer is pinned to the sniffed container and the codec to a short
    list, so a crafted upload cannot reach the rest of the media library, and
    the running length check means highly compressed audio cannot fill memory.
    """
    limit = (max_seconds + 1) * RATE
    chunks = []
    total = 0
    try:
        with av.open(io.BytesIO(body), format=container, metadata_errors="ignore") as source:
            stream = next((s for s in source.streams if s.type == "audio"), None)
            if stream is None or stream.codec_context.name not in CODECS:
                raise Rejected(400, "bad_audio")
            resampler = av.AudioResampler(format="flt", layout="mono", rate=RATE)
            for frame in source.decode(stream):
                for part in resampler.resample(frame):
                    samples = part.to_ndarray().reshape(-1).copy()
                    total += samples.size
                    if total > limit:
                        raise Rejected(413, "too_long")
                    chunks.append(samples)
            for part in resampler.resample(None):
                samples = part.to_ndarray().reshape(-1).copy()
                total += samples.size
                if total > limit:
                    raise Rejected(413, "too_long")
                chunks.append(samples)
    except (av.error.FFmpegError, ValueError, OSError, EOFError):
        raise Rejected(400, "bad_audio") from None
    if total == 0:
        raise Rejected(400, "bad_audio")
    return np.concatenate(chunks)


class Speech:
    """Admission, the concurrency limit and the loaded models."""

    def __init__(self, config):
        self.config = config
        self.lock = threading.Lock()
        self.load_lock = threading.Lock()
        self.admitted = 0
        self.slots = threading.Semaphore(config.concurrency)
        self.loaded = collections.OrderedDict()

    def enter(self):
        with self.lock:
            if self.admitted >= self.config.concurrency + self.config.queue:
                return False
            self.admitted += 1
            return True

    def leave(self):
        with self.lock:
            self.admitted -= 1

    def engine(self, language):
        with self.load_lock:
            found = self.loaded.get(language)
            if found is not None:
                self.loaded.move_to_end(language)
                return found
            spec, path = self.config.models[language]
            for other, value in self.loaded.items():
                if self.config.models[other] == (spec, path):
                    self.loaded[language] = value
                    return value
            while len(self.loaded) >= self.config.max_loaded:
                self.loaded.popitem(last=False)
            started = time.monotonic()
            engine = engines.resolve(spec)(path)
            log.info("loaded %s in %.1fs", language, time.monotonic() - started)
            self.loaded[language] = engine
            return engine

    def transcribe(self, body, language):
        container = container_of(body)
        if container is None:
            raise Rejected(400, "bad_audio")
        if not self.slots.acquire(timeout=self.config.queue_wait):
            raise Rejected(503, "busy")
        try:
            deadline = time.monotonic() + self.config.timeout
            samples = decode(body, container, self.config.max_seconds)
            seconds = samples.size / RATE
            if seconds > self.config.max_seconds + 0.5:
                raise Rejected(413, "too_long")
            started = time.monotonic()
            try:
                text = self.engine(language).transcribe(samples, language, deadline)
            except engines.TimedOut:
                raise Rejected(504, "timeout") from None
            took = time.monotonic() - started
            log.info("%s %.1fs audio in %.1fs", language, seconds, took)
            return text, seconds
        finally:
            self.slots.release()


class Handler(BaseHTTPRequestHandler):
    server_version = "stt"
    sys_version = ""
    protocol_version = "HTTP/1.1"
    timeout = 30

    def log_message(self, format, *args):
        pass

    def respond(self, status, payload):
        data = json.dumps(payload, ensure_ascii=False).encode()
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Connection", "close")
        self.end_headers()
        self.wfile.write(data)
        self.close_connection = True

    def authorized(self):
        secret = self.server.speech.config.secret
        if not secret:
            return True
        sent = self.headers.get("Authorization", "")
        return hmac.compare_digest(sent.encode(), f"Bearer {secret}".encode())

    def do_GET(self):
        if urlsplit(self.path).path != "/health":
            self.respond(404, {"error": "not_found"})
            return
        config = self.server.speech.config
        self.respond(200, {
            "ok": True,
            "languages": [lang for lang in LANGUAGES if lang in config.models],
            "max_seconds": config.max_seconds,
            "max_bytes": config.max_bytes,
        })

    def drain(self, length):
        left = length
        while left > 0:
            got = self.rfile.read(min(left, 65536))
            if not got:
                return
            left -= len(got)

    def do_POST(self):
        speech = self.server.speech
        length = self.headers.get("Content-Length", "")
        length = int(length) if length.isdigit() else -1
        if length < 0:
            self.respond(411, {"error": "length"})
            return
        url = urlsplit(self.path)
        language = parse_qs(url.query).get("language", [""])[0]
        problem = None
        if url.path != "/transcribe":
            problem = (404, "not_found")
        elif not self.authorized():
            problem = (401, "unauthorized")
        elif language not in speech.config.models:
            problem = (400, "language")
        elif length == 0:
            problem = (400, "bad_audio")
        elif length > speech.config.max_bytes:
            problem = (413, "too_large")
        elif not speech.enter():
            problem = (503, "busy")
        if problem:
            self.drain(length)
            self.respond(problem[0], {"error": problem[1]})
            return
        try:
            body = self.rfile.read(length)
            if len(body) != length:
                raise Rejected(400, "bad_audio")
            text, seconds = speech.transcribe(body, language)
            self.respond(200, {"text": text, "language": language, "seconds": round(seconds, 2)})
        except Rejected as why:
            self.respond(why.status, {"error": why.code})
        except (BrokenPipeError, ConnectionError, TimeoutError):
            self.close_connection = True
        except Exception as why:
            log.error("transcription failed: %s", type(why).__name__)
            self.respond(500, {"error": "failed"})
        finally:
            speech.leave()


def serve(config):
    server = ThreadingHTTPServer((config.host, config.port), Handler)
    server.daemon_threads = True
    server.speech = Speech(config)
    return server


def main():
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
    signal.signal(signal.SIGTERM, lambda *_: os._exit(0))
    config = Config()
    server = serve(config)
    if config.preload in config.models:
        threading.Thread(target=server.speech.engine, args=(config.preload,), daemon=True).start()
    log.info("listening on port %d for %s", config.port, ",".join(config.models) or "nothing")
    server.serve_forever()


if __name__ == "__main__":
    main()
