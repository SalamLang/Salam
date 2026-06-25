#!/usr/bin/env python3
import sys
from pathlib import Path

SKIP_DIRS = {".git", "node_modules", "dist", "build", ".venv", "vendor"}

# A `/` begins a regex literal (not division) when the previous significant
# token leaves us in expression position: after these punctuators, or after one
# of these keywords, or at the very start of input.
_REGEX_PREV_PUNCT = set("(,=:[!&|?{};~+-*/%<>^")
_REGEX_PREV_KEYWORDS = {
    "return", "typeof", "instanceof", "in", "of", "new", "delete",
    "void", "do", "else", "case", "yield", "await", "throw",
}

def _prev_token(out: list) -> str:
    """Last significant token already emitted (skipping trailing whitespace)."""
    k = len(out) - 1
    while k >= 0 and out[k] in " \t\r\n":
        k -= 1
    if k < 0:
        return ""
    if out[k].isalnum() or out[k] in "_$":
        end = k + 1
        while k >= 0 and (out[k].isalnum() or out[k] in "_$"):
            k -= 1
        return "".join(out[k + 1:end])
    return out[k]

def _regex_allowed(prev: str) -> bool:
    if not prev:
        return True
    if prev in _REGEX_PREV_KEYWORDS:
        return True
    return prev[-1] in _REGEX_PREV_PUNCT

def strip_comments(text: str, line_comments: bool, templates: bool) -> str:
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        nxt = text[i + 1] if i + 1 < n else ""
        if line_comments and c == "/" and nxt == "/":
            j = text.find("\n", i)
            if j == -1:
                break
            i = j
            continue
        if c == "/" and nxt == "*":
            j = text.find("*/", i + 2)
            if j == -1:
                break
            i = j + 2
            continue
        # Regex literal: consume it whole so an inner `/*`, `//` or quote is
        # never mistaken for a comment or string. Only when `/` is in
        # expression position (decided by the previous significant token).
        if line_comments and c == "/" and _regex_allowed(_prev_token(out)):
            out.append(c)
            i += 1
            in_class = False
            while i < n:
                ch = text[i]
                if ch == "\n":           # unterminated literal; stop consuming
                    break
                out.append(ch)
                if ch == "\\" and i + 1 < n:
                    out.append(text[i + 1])
                    i += 2
                    continue
                i += 1
                if ch == "[":
                    in_class = True
                elif ch == "]":
                    in_class = False
                elif ch == "/" and not in_class:
                    break
            continue
        if c == '"' or c == "'" or (templates and c == "`"):
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
    root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path.cwd()
    if not root.is_dir():
        print(f"error: {root} is not a directory", file=sys.stderr)
        return 1
    changed = 0
    for path in sorted(root.rglob("*")):
        if path.suffix not in (".css", ".js"):
            continue
        if any(part in SKIP_DIRS for part in path.parts):
            continue
        original = path.read_text(encoding="utf-8")
        is_js = path.suffix == ".js"
        stripped = strip_comments(original, line_comments=is_js, templates=is_js)
        if stripped != original:
            path.write_text(stripped, encoding="utf-8")
            changed += 1
            print(f"stripped: {path.relative_to(root)}")
    print(f"\nDone. Modified {changed} file(s).")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
