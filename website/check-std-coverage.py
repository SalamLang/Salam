#!/usr/bin/env python3
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SKIP = {
    "console",
    "layout",
    "layoutgen",
    "map",
    "net",
    "net/internal",
    "web",
    "telegram",
}
SKIP_PREFIX = ("layout/", "net/internal/")


def pages(lang):
    found = set()
    directory = os.path.join(ROOT, "website", "content", lang, "std")
    for name in sorted(os.listdir(directory)):
        if not name.endswith(".txt"):
            continue
        path = os.path.join(directory, name)
        with open(path, encoding="utf-8") as f:
            for line in f:
                if line.startswith("pkg.en = "):
                    found.add(line.split("=", 1)[1].strip())
                    break
    return found


def packages():
    found = set()
    base = os.path.join(ROOT, "std")
    for root, _, files in os.walk(base):
        if not any(name.endswith(".salam") for name in files):
            continue
        rel = os.path.relpath(root, base).replace(os.sep, "/")
        if rel == ".":
            continue
        text = ""
        for name in files:
            if name.endswith(".salam"):
                with open(os.path.join(root, name), encoding="utf-8") as f:
                    text += f.read()
        if "package " not in text:
            continue
        if rel in SKIP or rel.startswith(SKIP_PREFIX):
            continue
        found.add(rel)
    return found


en = pages("en")
fa = pages("fa")
expected = packages()

missing_en = sorted(expected - en)
missing_fa = sorted(expected - fa)
missing_translation = sorted(en ^ fa)

if missing_en:
    print(
        "website: std packages without an English page:",
        ", ".join(missing_en),
        file=sys.stderr,
    )
if missing_fa:
    print(
        "website: std packages without a Persian page:",
        ", ".join(missing_fa),
        file=sys.stderr,
    )
if missing_translation:
    print(
        "website: std pages only in one language:",
        ", ".join(missing_translation),
        file=sys.stderr,
    )
if missing_en or missing_fa or missing_translation:
    sys.exit(1)

print(
    f"std coverage: {len(expected)} packages, {len(en)} English and {len(fa)} Persian pages"
)
