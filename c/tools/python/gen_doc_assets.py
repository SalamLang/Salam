#!/usr/bin/env python3
"""Regenerate c/src/doc/doc_assets.c from compiler/docgen.salam.

`salam doc` exists twice: once in the self-hosted compiler (compiler/docgen.salam)
and once in this C toolchain (c/src/doc/). The rendering logic is ported by hand,
but the three static assets - the stylesheet, the viewer script, and the body
skeleton - are pure data and must stay byte-identical, so they are generated
rather than transcribed.

In docgen.salam the assets are backtick-delimited raw string literals, in which
this language performs no escape decoding at all (see lexer.salam's
scan_raw_string). This script therefore reads them verbatim and re-encodes each
source line as one C string literal.

Usage (from the repository root):

    python c/tools/python/gen_doc_assets.py
"""

import io
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
SRC = os.path.join(ROOT, "compiler", "docgen.salam")
OUT = os.path.join(ROOT, "c", "src", "doc", "doc_assets.c")

HEADER = """/*
 * Salam Programming Language (2024-2026)
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

/*
 * Static HTML assets for `salam doc`: the inline stylesheet, the inline viewer
 * script, and the body skeleton that script renders into. Byte-for-byte the
 * same assets the self-hosted compiler/docgen.salam carries as backtick
 * raw-string literals - the two must stay in sync.
 *
 * GENERATED FILE - do not hand-edit. Regenerate after changing the assets in
 * compiler/docgen.salam with:
 *
 *     python c/tools/python/gen_doc_assets.py
 *
 * One C string literal per source line, rather than one huge concatenated
 * literal, keeps every literal well inside C90's guaranteed 509-character
 * limit and keeps the diff against the .salam original line-addressable.
 */

#include "core/prelude.h"
#include "doc/docgen.h"

"""


def read_raw(text, name):
    """Return the backtick raw-string literal assigned to `name`."""
    key = name + " := `"
    try:
        i = text.index(key) + len(key)
        j = text.index("`", i)
    except ValueError:
        sys.exit(f"gen_doc_assets: cannot find raw string '{name}' in {SRC}")
    return text[i:j]


def c_literal(line):
    """Encode one source line as a C string literal, newline included."""
    out = ['"']
    for ch in line:
        if ch == "\\":
            out.append("\\\\")
        elif ch == '"':
            out.append('\\"')
        elif ch == "\t":
            out.append("\\t")
        elif ch == "?":
            # '??x' would be read as a trigraph by a strict C90 compiler.
            out.append("\\?")
        else:
            out.append(ch)
    out.append('\\n"')
    return "".join(out)


def emit_array(w, cname, s):
    lines = s.split("\n")
    # Every asset ends with a newline; c_literal() re-adds it to the last real
    # line, so drop the empty tail split() leaves behind.
    if lines and lines[-1] == "":
        lines.pop()
    w.write("const char *const %s[] = {\n" % cname)
    for line in lines:
        w.write(f"    {c_literal(line)},\n")
    w.write("    NULL\n};\n\n")


def main():
    with io.open(SRC, encoding="utf-8", newline="\n") as src:
        text = src.read()
    w = io.StringIO()
    w.write(HEADER)
    emit_array(w, "doc_css_lines", read_raw(text, "css_text"))
    emit_array(w, "doc_js_lines", read_raw(text, "js_text"))
    emit_array(w, "doc_body_lines", read_raw(text, "body_text"))
    with io.open(OUT, "w", encoding="utf-8", newline="\n") as out:
        out.write(w.getvalue())
    sys.stdout.write(f"wrote {os.path.relpath(OUT, ROOT)}\n")


if __name__ == "__main__":
    main()
