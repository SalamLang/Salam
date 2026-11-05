"""Measure one model: real-time factor, peak memory and, with references, WER and CER.

    python bench.py --model /models/x --audio clips/ --language fa [--refs refs.tsv]

The references are a UTF-8 file of "file name<TAB>text" lines. Audio can be in
any format the media library reads. Run one model per process, so that the peak
memory belongs to that model. The engine comes from --engine or the same
environment settings the service uses (STT_ENGINE, STT_RUNTIME, STT_LOAD_OPTIONS,
STT_RUN_OPTIONS).
"""

import argparse
import json
import os
import re
import resource
import time
import unicodedata

import av
import numpy as np

import engines

RATE = 16000
FOLD = str.maketrans({"ي": "ی", "ى": "ی", "ك": "ک", "ۀ": "ه", "ة": "ه", "أ": "ا", "إ": "ا", "آ": "ا", "ؤ": "و", "‌": " "})
DIGITS = str.maketrans("۰۱۲۳۴۵۶۷۸۹٠١٢٣٤٥٦٧٨٩", "01234567890123456789")


def normalize(text):
    text = unicodedata.normalize("NFKC", text).translate(FOLD).translate(DIGITS).lower()
    text = "".join(c for c in text if unicodedata.category(c) != "Mn")
    text = "".join(" " if unicodedata.category(c)[0] in "PSZC" else c for c in text)
    return re.sub(r"\s+", " ", text).strip()


def edits(a, b):
    row = list(range(len(b) + 1))
    for i, x in enumerate(a, 1):
        last, row[0] = row[0], i
        for j, y in enumerate(b, 1):
            last, row[j] = row[j], min(row[j] + 1, row[j - 1] + 1, last + (x != y))
    return row[-1]


def read_audio(path):
    chunks = []
    with av.open(path) as source:
        stream = next(s for s in source.streams if s.type == "audio")
        resampler = av.AudioResampler(format="flt", layout="mono", rate=RATE)
        for frame in source.decode(stream):
            chunks.extend(p.to_ndarray().reshape(-1).copy() for p in resampler.resample(frame))
        chunks.extend(p.to_ndarray().reshape(-1).copy() for p in resampler.resample(None))
    return np.concatenate(chunks)


def peak_mb():
    return resource.getrusage(resource.RUSAGE_SELF).ru_maxrss / 1024


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--model", required=True)
    parser.add_argument("--audio", required=True)
    parser.add_argument("--language", required=True)
    parser.add_argument("--refs")
    parser.add_argument("--engine", default=os.environ.get("STT_ENGINE", "engines:SegmentEngine"))
    parser.add_argument("--show", action="store_true", help="print each hypothesis")
    parser.add_argument("--json", help="also write the summary to this file")
    args = parser.parse_args()

    refs = {}
    if args.refs:
        with open(args.refs, encoding="utf-8") as lines:
            for line in lines:
                name, _, text = line.rstrip("\n").partition("\t")
                if text:
                    refs[name] = text

    started = time.monotonic()
    engine = engines.resolve(args.engine)(args.model)
    load_seconds = time.monotonic() - started
    load_peak = peak_mb()

    audio_seconds = 0.0
    work_seconds = 0.0
    word_edits = word_total = char_edits = char_total = 0
    worst_rtf = 0.0
    names = sorted(n for n in os.listdir(args.audio) if os.path.isfile(os.path.join(args.audio, n)))
    for name in names:
        samples = read_audio(os.path.join(args.audio, name))
        seconds = samples.size / RATE
        begun = time.monotonic()
        text = engine.transcribe(samples, args.language, time.monotonic() + 3600)
        took = time.monotonic() - begun
        audio_seconds += seconds
        work_seconds += took
        worst_rtf = max(worst_rtf, took / seconds)
        line = f"{name}\t{seconds:.1f}s\trtf {took / seconds:.2f}"
        if name in refs:
            ref = normalize(refs[name])
            hyp = normalize(text)
            word_edits += edits(ref.split(), hyp.split())
            word_total += len(ref.split())
            char_edits += edits(ref.replace(" ", ""), hyp.replace(" ", ""))
            char_total += len(ref.replace(" ", ""))
            line += f"\twer {100 * edits(ref.split(), hyp.split()) / max(1, len(ref.split())):.1f}"
        print(line + (f"\t{text}" if args.show else ""), flush=True)

    summary = {
        "files": len(names),
        "audio_seconds": round(audio_seconds, 1),
        "load_seconds": round(load_seconds, 1),
        "rtf": round(work_seconds / audio_seconds, 3) if audio_seconds else None,
        "worst_rtf": round(worst_rtf, 3),
        "peak_mb_after_load": round(load_peak),
        "peak_mb": round(peak_mb()),
        "cpus": os.cpu_count(),
        "load_average": [round(x, 1) for x in os.getloadavg()],
    }
    if word_total:
        summary["wer"] = round(100 * word_edits / word_total, 2)
        summary["cer"] = round(100 * char_edits / char_total, 2)
        summary["reference_words"] = word_total
    print(json.dumps(summary, indent=2))
    if args.json:
        with open(args.json, "w", encoding="utf-8") as out:
            json.dump(summary, out, indent=2)


if __name__ == "__main__":
    main()
