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


class SegmentEngine:
    """For runtimes whose model class takes a path and yields text segments.

    The class comes from STT_RUNTIME ("module:Class"). Its constructor gets
    STT_LOAD_OPTIONS and its transcribe call gets STT_RUN_OPTIONS, both JSON.
    """

    def __init__(self, model_path):
        runtime = os.environ.get("STT_RUNTIME", "")
        self.model = resolve(runtime)(model_path, **json_env("STT_LOAD_OPTIONS"))
        self.options = json_env("STT_RUN_OPTIONS")

    def transcribe(self, samples, language, deadline):
        result = self.model.transcribe(samples, language=language, **self.options)
        segments = result[0] if isinstance(result, tuple) else result
        parts = []
        for segment in segments:
            if time.monotonic() > deadline:
                raise TimedOut()
            parts.append(segment.text)
        return "".join(parts).strip()


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
