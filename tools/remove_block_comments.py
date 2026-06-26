#!/usr/bin/env python3
import sys
from pathlib import Path

SRC_DIR = Path(__file__).parent / "src"

def strip_block_comments(text: str) -> str:
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        nxt = text[i + 1] if i + 1 < n else ""
        if c == "/" and nxt == "/":
            j = text.find("\n", i)
            if j == -1:
                out.append(text[i:])
                break
            out.append(text[i:j])
            i = j
            continue
        if c == "/" and nxt == "*":
            j = text.find("*/", i + 2)
            if j == -1:
                break
            i = j + 2
            continue
        if c == '"' or c == "'":
            quote = c
            out.append(c)
            i += 1
            while i < n:
                ch = text[i]
                out.append(ch)
                if ch == "\\" and i + 1 < n:
                    out.append(text[i + 1])
                    i += 2
                    continue
                i += 1
                if ch == quote:
                    break
            continue
        out.append(c)
        i += 1
    return "".join(out)

def main() -> int:
    if not SRC_DIR.is_dir():
        print(f"error: {SRC_DIR} not found", file=sys.stderr)
        return 1
    changed = 0
    for path in sorted(SRC_DIR.rglob("*.c")):
        original = path.read_text(encoding="utf-8")
        stripped = strip_block_comments(original)
        if stripped != original:
            path.write_text(stripped, encoding="utf-8")
            changed += 1
            print(f"stripped: {path.relative_to(SRC_DIR.parent)}")
    print(f"\nDone. Modified {changed} file(s).")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
