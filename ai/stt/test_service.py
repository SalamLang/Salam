"""Tests for the speech service with the stub engine: python -m unittest test_service"""

import http.client
import io
import json
import os
import threading
import time
import unittest

import av
import numpy as np

os.environ["STT_ENGINE"] = "engines:StubEngine"

import engines  # noqa: E402
import service  # noqa: E402

RATE = 48000


def encode(seconds, container, codec, frame_size=960, options=None):
    out = io.BytesIO()
    with av.open(out, mode="w", format=container, options=options or {}) as target:
        stream = target.add_stream(codec, rate=RATE, layout="mono")
        stream.bit_rate = 16000
        for i in range(int(seconds * RATE / frame_size)):
            t = (np.arange(frame_size) + i * frame_size) / RATE
            data = (np.sin(2 * np.pi * 440 * t) * 8000).astype(np.int16).reshape(1, -1)
            frame = av.AudioFrame.from_ndarray(data, format="s16", layout="mono")
            frame.sample_rate = RATE
            frame.pts = i * frame_size
            for packet in stream.encode(frame):
                target.mux(packet)
        for packet in stream.encode(None):
            target.mux(packet)
    return out.getvalue()


def make_config(**over):
    config = service.Config()
    config.host = "127.0.0.1"
    config.port = 0
    config.secret = ""
    config.max_seconds = 5
    config.max_bytes = 200000
    config.concurrency = 1
    config.queue = 1
    config.queue_wait = 5
    config.timeout = 10
    config.models = {"fa": ("engines:StubEngine", "/models/a"), "en": ("engines:StubEngine", "/models/b")}
    for key, value in over.items():
        setattr(config, key, value)
    return config


class Running:
    def __init__(self, **over):
        self.server = service.serve(make_config(**over))
        self.port = self.server.server_address[1]
        threading.Thread(target=self.server.serve_forever, daemon=True).start()

    def request(self, method, path, body=None, headers=None):
        conn = http.client.HTTPConnection("127.0.0.1", self.port, timeout=30)
        conn.request(method, path, body=body, headers=headers or {})
        res = conn.getresponse()
        data = json.loads(res.read())
        conn.close()
        return res.status, data

    def close(self):
        self.server.shutdown()
        self.server.server_close()


class ServiceTest(unittest.TestCase):
    def setUp(self):
        self.addCleanup(lambda: os.environ.pop("STT_STUB_SECONDS", None))

    def running(self, **over):
        run = Running(**over)
        self.addCleanup(run.close)
        return run

    def test_health_reports_only_languages_and_limits(self):
        status, data = self.running().request("GET", "/health")
        self.assertEqual(status, 200)
        self.assertEqual(data, {"ok": True, "languages": ["fa", "en"], "max_seconds": 5, "max_bytes": 200000})

    def test_only_configured_languages_are_offered(self):
        run = self.running(models={"fa": ("engines:StubEngine", "/models/a")})
        self.assertEqual(run.request("GET", "/health")[1]["languages"], ["fa"])
        status, data = run.request("POST", "/transcribe?language=en", body=encode(1, "webm", "libopus"))
        self.assertEqual((status, data["error"]), (400, "language"))

    def test_every_browser_format_decodes(self):
        run = self.running()
        fragmented = {"movflags": "frag_keyframe+empty_moov+default_base_moof"}
        for container, codec, frame, options in (("webm", "libopus", 960, None), ("ogg", "libopus", 960, None), ("mp4", "aac", 1024, None), ("mp4", "aac", 1024, fragmented)):
            body = encode(1, container, codec, frame, options)
            status, data = run.request("POST", "/transcribe?language=fa", body=body)
            self.assertEqual(status, 200, container)
            self.assertTrue(data["text"].startswith("stub fa "), container)
            self.assertEqual(data["language"], "fa")
            self.assertAlmostEqual(data["seconds"], 1, delta=0.2)
            self.assertEqual(set(data), {"text", "language", "seconds"})

    def test_music_and_wav_formats_decode(self):
        run = self.running()
        for container, codec, options in (("mp3", "libmp3lame", None), ("flac", "flac", None), ("wav", "pcm_s16le", None), ("wav", "pcm_s24le", None)):
            body = encode(1, container, codec, 1152 if container == "mp3" else 960, options)
            self.assertEqual(service.container_of(body), container, codec)
            status, data = run.request("POST", "/transcribe?language=en", body=body)
            self.assertEqual(status, 200, codec)
            self.assertTrue(data["text"].startswith("stub en "), codec)
            self.assertAlmostEqual(data["seconds"], 1, delta=0.3)

    def test_mp3_with_a_tag_and_without_one_are_both_recognised(self):
        body = encode(1, "mp3", "libmp3lame", 1152)
        tagged = b"ID3\x04\x00\x00\x00\x00\x00\x00" + body
        self.assertEqual(service.container_of(tagged), "mp3")
        bare = body[body.index(b"\xff"):] if body[:3] == b"ID3" else body
        self.assertEqual(service.container_of(bare), "mp3")
        status, data = self.running().request("POST", "/transcribe?language=fa", body=tagged)
        self.assertEqual(status, 200)

    def test_sniffing_rejects_lookalikes(self):
        pad = bytes(32)
        for head in (b"RIFF\x00\x00\x00\x00AVI ", b"RIFF\x00\x00\x00\x00WAVX", b"\xff\xf1\x50\x80", b"\xff\xe2\xff\x00", b"\xff\xfb\xf0\x00", b"\xff\xfb\x00\x00", b"fLaX", b"ID"):
            self.assertIsNone(service.container_of(head + pad), head)

    def test_unlisted_codecs_in_known_containers_are_refused(self):
        run = self.running()
        body = encode(1, "wav", "pcm_alaw")
        status, data = run.request("POST", "/transcribe?language=fa", body=body)
        self.assertEqual((status, data["error"]), (400, "bad_audio"))

    def test_long_wav_stops_at_the_limit(self):
        body = encode(8, "wav", "pcm_s16le")
        status, data = self.running(max_bytes=10_000_000).request("POST", "/transcribe?language=fa", body=body)
        self.assertEqual((status, data["error"]), (413, "too_long"))

    def test_truncated_music_files_are_refused(self):
        run = self.running()
        for container, codec in (("flac", "flac"), ("wav", "pcm_s16le"), ("mp3", "libmp3lame")):
            body = encode(1, container, codec)
            status, data = run.request("POST", "/transcribe?language=fa", body=body[:20])
            self.assertEqual((status, data["error"]), (400, "bad_audio"), container)

    def test_language_goes_to_its_own_engine(self):
        status, data = self.running().request("POST", "/transcribe?language=en", body=encode(1, "webm", "libopus"))
        self.assertEqual(status, 200)
        self.assertTrue(data["text"].startswith("stub en "))

    def test_bad_uploads_are_refused(self):
        run = self.running()
        good = encode(1, "webm", "libopus")
        for body in (b"not audio at all, just some text", b"\x89PNG\r\n\x1a\n" + bytes(64), good[:40], b"OggS" + bytes(64)):
            status, data = run.request("POST", "/transcribe?language=fa", body=body)
            self.assertEqual((status, data["error"]), (400, "bad_audio"), body[:8])
        status, data = run.request("POST", "/transcribe?language=fa", body=b"")
        self.assertEqual((status, data["error"]), (400, "bad_audio"))

    def test_too_long_stops_at_the_limit(self):
        body = encode(8, "webm", "libopus")
        status, data = self.running(max_bytes=10_000_000).request("POST", "/transcribe?language=fa", body=body)
        self.assertEqual((status, data["error"]), (413, "too_long"))

    def test_too_large_is_refused_and_the_body_drained(self):
        status, data = self.running().request("POST", "/transcribe?language=fa", body=bytes(300000))
        self.assertEqual((status, data["error"]), (413, "too_large"))

    def test_busy_is_a_clean_rejection(self):
        os.environ["STT_STUB_SECONDS"] = "1.5"
        run = self.running()
        body = encode(1, "webm", "libopus")
        results = []
        def post():
            results.append(run.request("POST", "/transcribe?language=fa", body=body)[0])
        threads = [threading.Thread(target=post) for _ in range(2)]
        for thread in threads:
            thread.start()
        time.sleep(0.3)
        status, data = run.request("POST", "/transcribe?language=fa", body=body)
        self.assertEqual((status, data["error"]), (503, "busy"))
        for thread in threads:
            thread.join()
        self.assertEqual(results, [200, 200])
        self.assertEqual(run.request("POST", "/transcribe?language=fa", body=body)[0], 200)

    def test_queue_wait_is_bounded(self):
        os.environ["STT_STUB_SECONDS"] = "2"
        run = self.running(queue_wait=1)
        body = encode(1, "webm", "libopus")
        slow = threading.Thread(target=run.request, args=("POST", "/transcribe?language=fa"), kwargs={"body": body})
        slow.start()
        time.sleep(0.3)
        status, data = run.request("POST", "/transcribe?language=fa", body=body)
        slow.join()
        self.assertEqual((status, data["error"]), (503, "busy"))

    def test_timeout_answers_504(self):
        os.environ["STT_STUB_SECONDS"] = "5"
        status, data = self.running(timeout=1).request("POST", "/transcribe?language=fa", body=encode(1, "webm", "libopus"))
        self.assertEqual((status, data["error"]), (504, "timeout"))

    def test_engine_errors_do_not_leak_details(self):
        run = self.running(models={"fa": ("engines:Missing", "/models/secret-name")})
        status, data = run.request("POST", "/transcribe?language=fa", body=encode(1, "webm", "libopus"))
        self.assertEqual(status, 500)
        self.assertEqual(data, {"error": "failed"})

    def test_secret_is_enforced_when_set(self):
        run = self.running(secret="s3cret")
        body = encode(1, "webm", "libopus")
        self.assertEqual(run.request("POST", "/transcribe?language=fa", body=body)[0], 401)
        ok = run.request("POST", "/transcribe?language=fa", body=body, headers={"Authorization": "Bearer s3cret"})
        self.assertEqual(ok[0], 200)

    def test_models_stay_within_the_loaded_limit(self):
        run = self.running(max_loaded=1)
        body = encode(1, "webm", "libopus")
        run.request("POST", "/transcribe?language=fa", body=body)
        run.request("POST", "/transcribe?language=en", body=body)
        self.assertEqual(list(run.server.speech.loaded), ["en"])

    def test_segment_engine_stops_at_the_deadline(self):
        class Segment:
            text = " word"

        class Runtime:
            def __init__(self, path):
                self.path = path

            def transcribe(self, samples, language):
                while True:
                    yield Segment()

        engines.Runtime = Runtime
        os.environ["STT_RUNTIME"] = "engines:Runtime"
        self.addCleanup(lambda: os.environ.pop("STT_RUNTIME", None))
        engine = engines.SegmentEngine("/models/a")
        with self.assertRaises(engines.TimedOut):
            engine.transcribe(np.zeros(10), "fa", time.monotonic() + 0.05)


if __name__ == "__main__":
    unittest.main()
