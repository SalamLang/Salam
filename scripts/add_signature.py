#!/usr/bin/env python3
# Prepend the Salam signature banner to every .c and .h source file.

import argparse
import os
import sys

SIGNATURE = """/*
 * Salam Programming Language (2024–2026)
 *
 *   +-------------------+
 *   |     S A L A M     |
 *   +-------------------+
 *
 * Designed by Seyyed Ali Mohammadiyeh and the Salam Team
 * Born from a decade of language design experience (since 2018)
 *
 * Repository: https://github.com/SalamLang/Salam
 *
 */
"""

MARKER = "Salam Programming Language (2024–2026)"

SKIP_DIRS = {".git", "build", "out", "node_modules", "third_party", "vendor"}


def has_signature(content: str) -> bool:
    """Return True if the file already begins with the banner."""
    head = content[:len(SIGNATURE) + 50].lstrip("﻿").lstrip()
    return head.startswith("/*") and MARKER in head


def build_new_content(content: str) -> str:
    """Return content with the signature prepended and one blank line after."""
    bom = ""
    if content.startswith("﻿"):
        bom = "﻿"
        content = content[1:]

    body = content.lstrip("\n")
    return bom + SIGNATURE + "\n" + body


def process_file(path: str, check_only: bool) -> bool:
    """Process a single file. Return True if it changed (or would change)."""
    try:
        with open(path, "r", encoding="utf-8") as f:
            content = f.read()
    except (OSError, UnicodeDecodeError) as exc:
        print(f"  skip (cannot read): {path} ({exc})", file=sys.stderr)
        return False

    if has_signature(content):
        return False

    if check_only:
        print(f"  would add: {path}")
        return True

    new_content = build_new_content(content)
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(new_content)
    print(f"  added: {path}")
    return True


def iter_source_files(root: str):
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]
        for name in filenames:
            if name.endswith((".c", ".h")):
                yield os.path.join(dirpath, name)


def main() -> int:
    parser = argparse.ArgumentParser(description="Prepend the Salam signature to .c/.h files.")
    parser.add_argument("root", nargs="?", default=".", help="Directory to process (default: current dir).")
    parser.add_argument("--check", action="store_true", help="Report files that would change without modifying them.")
    args = parser.parse_args()

    root = os.path.abspath(args.root)
    if not os.path.isdir(root):
        print(f"error: not a directory: {root}", file=sys.stderr)
        return 2

    changed = 0
    total = 0
    for path in iter_source_files(root):
        total += 1
        if process_file(path, args.check):
            changed += 1

    verb = "would be updated" if args.check else "updated"
    print(f"\nDone. {changed} of {total} file(s) {verb}.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
