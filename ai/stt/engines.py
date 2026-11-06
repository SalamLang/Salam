"""Speech engines. An engine turns 16 kHz mono float32 samples into text.

Which engine serves a language is an import spec ("module:factory") read from
the environment, so another kind of model only needs a new module on the
PYTHONPATH and a server setting. A factory takes the model path and returns an
object with transcribe(samples, language, deadline) -> str.
"""

import importlib
import json
import os
import time

import numpy as np

RATE = 16000
SMOOTH_FRAMES = 15


class TimedOut(Exception):
    pass


def resolve(spec):
    module, _, name = spec.partition(":")
    if not module or not name:
        raise ValueError("an engine spec looks like module:name")
    return getattr(importlib.import_module(module), name)


def json_env(name):
    raw = os.environ.get(name, "").strip()
    return json.loads(raw) if raw else {}


def split_at_quiet(samples, max_seconds):
    """Cut samples into pieces of at most max_seconds, at the quietest moment
    in the second half of each piece, so that a word is rarely cut in two."""
    longest = max(int(max_seconds * RATE), RATE)
    frame = int(0.02 * RATE)
    pieces = []
    start = 0
    while samples.size - start > longest:
        first = start + longest // 2
        window = samples[first : start + longest]
        count = window.size // frame
        energy = np.square(window[: count * frame]).reshape(count, frame).mean(axis=1)
        smooth = np.convolve(energy, np.ones(SMOOTH_FRAMES) / SMOOTH_FRAMES, mode="valid")
        cut = first + (int(np.argmin(smooth)) + SMOOTH_FRAMES // 2) * frame
        pieces.append((start, cut))
        start = cut
    pieces.append((start, samples.size))
    return pieces


class SegmentEngine:
    """For runtimes whose model class takes a path and yields text segments.

    The class comes from STT_RUNTIME ("module:Class"). Its constructor gets
    STT_LOAD_OPTIONS and its transcribe call gets STT_RUN_OPTIONS, both JSON.
    A model trained on short utterances can stop after its first sentence and
    lose the rest of a long clip; STT_SPLIT_SECONDS decodes such a clip in
    pieces of at most that length instead.
    """

    def __init__(self, model_path):
        runtime = os.environ.get("STT_RUNTIME", "")
        self.model = resolve(runtime)(model_path, **json_env("STT_LOAD_OPTIONS"))
        self.options = json_env("STT_RUN_OPTIONS")
        self.split = float(os.environ.get("STT_SPLIT_SECONDS", "") or 0)

    def decode(self, samples, language, deadline):
        result = self.model.transcribe(samples, language=language, **self.options)
        segments = result[0] if isinstance(result, tuple) else result
        parts = []
        for segment in segments:
            if time.monotonic() > deadline:
                raise TimedOut()
            parts.append(segment.text)
        return "".join(parts).strip()

    def transcribe(self, samples, language, deadline):
        pieces = split_at_quiet(samples, self.split) if self.split > 0 else [(0, samples.size)]
        texts = []
        for start, end in pieces:
            if time.monotonic() > deadline:
                raise TimedOut()
            texts.append(self.decode(samples[start:end], language, deadline))
        return " ".join(text for text in texts if text)


class StubEngine:
    """Answers without a model, for tests. STT_STUB_SECONDS makes it slow."""

    def __init__(self, model_path):
        self.model_path = model_path
        self.pause = float(os.environ.get("STT_STUB_SECONDS", "0"))

    def transcribe(self, samples, language, deadline):
        end = time.monotonic() + self.pause
        while time.monotonic() < end:
            if time.monotonic() > deadline:
                raise TimedOut()
            time.sleep(0.01)
        return f"stub {language} {len(samples)}"
