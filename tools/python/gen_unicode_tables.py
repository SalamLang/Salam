#!/usr/bin/env python3
"""Regenerate std/unicode/tables.salam from the Unicode Character Database.

std/unicode is a data package: every table in tables.salam is derived
mechanically from the UCD text files listed in FILES below, so the file is
generated rather than hand-maintained. Editing tables.salam directly is a
mistake - change this script and re-run it instead.

The four inputs and what each one contributes:

    UnicodeData.txt           general category, simple upper/lower/title
    CaseFolding.txt           simple (C+S) and full (C+F) case folding
    PropList.txt              White_Space
    DerivedCoreProperties.txt Alphabetic

Everything is emitted as sorted range tables so lookups are a binary search
over a few thousand entries rather than a 1.1M-entry array. Categories are
(lo, hi, id) triples covering only assigned code points - a code point that
falls in no range is unassigned (Cn). The case tables use the (lo, hi, stride,
delta) form: within one range every mapping shifts the code point by the same
delta, and `stride` is 1 for a solid run or 2 for the interleaved upper/lower
pairs that fill much of Latin Extended and Cyrillic. That compression takes
~1450 individual mappings down to ~200 ranges each.

Case mapping here is the default, non-tailored kind. Locale-specific rules
(Turkish dotless i, Lithuanian accented i, Azeri) are deliberately not applied;
a caller that needs them has to tailor on top.

Usage (from the repository root):

    python tools/python/gen_unicode_tables.py            # use cached UCD files
    python tools/python/gen_unicode_tables.py --fetch    # download them first

The cache lives in tools/python/.ucd-cache/ and is not checked in. The UCD
version is pinned by UCD_VERSION; bumping it and re-running with --fetch is the
whole upgrade procedure.
"""

import argparse
import os
import sys
import urllib.request

UCD_VERSION = "16.0.0"
UCD_BASE = "https://www.unicode.org/Public/%s/ucd" % UCD_VERSION
FILES = (
    "UnicodeData.txt",
    "CaseFolding.txt",
    "PropList.txt",
    "DerivedCoreProperties.txt",
)

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
CACHE = os.path.join(HERE, ".ucd-cache")
OUT = os.path.join(ROOT, "std", "unicode", "tables.salam")

MAX_CP = 0x110000

# General category ids. The order is not arbitrary: categories that share a
# first letter are numbered consecutively so that every group predicate in
# unicode.salam is a range check on the id (IsLetter is `id <= CatLo`, IsMark
# is `id >= CatMn && id <= CatMe`, and so on) instead of a chain of equality
# tests. Cn sorts last so that "unassigned" is also the largest id.
CATEGORIES = (
    "Lu", "Ll", "Lt", "Lm", "Lo",
    "Mn", "Mc", "Me",
    "Nd", "Nl", "No",
    "Pc", "Pd", "Ps", "Pe", "Pi", "Pf", "Po",
    "Sm", "Sc", "Sk", "So",
    "Zs", "Zl", "Zp",
    "Cc", "Cf", "Cs", "Co", "Cn",
)
CAT_ID = {name: i for i, name in enumerate(CATEGORIES)}

BANNER = """/*
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
"""


# --------------------------------------------------------------------------
# input
# --------------------------------------------------------------------------

def ucd_path(name):
    return os.path.join(CACHE, name)


def fetch():
    os.makedirs(CACHE, exist_ok=True)
    for name in FILES:
        url = "%s/%s" % (UCD_BASE, name)
        sys.stderr.write("fetching %s\n" % url)
        with urllib.request.urlopen(url) as r:
            data = r.read()
        with open(ucd_path(name), "wb") as f:
            f.write(data)


def read_lines(name):
    path = ucd_path(name)
    if not os.path.exists(path):
        sys.exit(
            "missing %s\nRun with --fetch to download the UCD %s files."
            % (path, UCD_VERSION)
        )
    with open(path, encoding="utf-8") as f:
        for line in f:
            line = line.split("#", 1)[0].strip()
            if line:
                yield line


def parse_unicode_data():
    """-> (categories, upper, lower, title)

    categories is a {cp: category} style range list built as (lo, hi, cat)
    triples; the case maps are {cp: mapped_cp} dicts of the simple mappings.

    UnicodeData.txt lists large uniform blocks (CJK, Hangul, the private use
    areas) as a First/Last pair of lines rather than one line per code point,
    so those have to be expanded into a range instead of read literally.
    """
    ranges = []
    upper, lower, title = {}, {}, {}
    pending_first = None

    for line in read_lines("UnicodeData.txt"):
        f = line.split(";")
        cp = int(f[0], 16)
        name, cat = f[1], f[2]

        if name.endswith(", First>"):
            pending_first = (cp, cat)
            continue
        if name.endswith(", Last>"):
            if pending_first is None or pending_first[1] != cat:
                sys.exit("unpaired range terminator at U+%04X" % cp)
            ranges.append((pending_first[0], cp, cat))
            pending_first = None
            continue

        ranges.append((cp, cp, cat))
        if f[12]:
            upper[cp] = int(f[12], 16)
        if f[13]:
            lower[cp] = int(f[13], 16)
        if f[14]:
            title[cp] = int(f[14], 16)

    if pending_first is not None:
        sys.exit("unterminated range starting at U+%04X" % pending_first[0])

    return merge_cat_ranges(ranges), upper, lower, title


def merge_cat_ranges(ranges):
    """Coalesce adjacent same-category ranges. UnicodeData.txt is already in
    code point order, so a single forward pass is enough."""
    ranges.sort()
    out = []
    for lo, hi, cat in ranges:
        if out and out[-1][2] == cat and out[-1][1] + 1 == lo:
            out[-1] = (out[-1][0], hi, cat)
        else:
            out.append((lo, hi, cat))
    return out


def parse_case_folding():
    """-> (simple, full)

    simple maps cp -> one cp, using the C (common) and S (simple) statuses.
    full maps cp -> a string of one or more cps, using C and F; only the
    entries that actually expand to more than one code point are kept, since
    the single-code-point ones are already covered by `simple`.

    T (Turkic) entries are skipped - see the module docstring on tailoring.
    """
    simple, full = {}, {}
    for line in read_lines("CaseFolding.txt"):
        f = [p.strip() for p in line.split(";")]
        cp, status, mapping = int(f[0], 16), f[1], f[2]
        cps = [int(x, 16) for x in mapping.split()]
        if status in ("C", "S"):
            if len(cps) != 1:
                sys.exit("non-simple mapping for status %s at U+%04X" % (status, cp))
            simple[cp] = cps[0]
        if status in ("C", "F") and len(cps) > 1:
            full[cp] = cps
    return simple, full


def parse_prop(filename, wanted):
    """Code point ranges carrying property `wanted` in a PropList-style file."""
    out = []
    for line in read_lines(filename):
        f = [p.strip() for p in line.split(";")]
        if len(f) < 2 or f[1] != wanted:
            continue
        span = f[0].split("..")
        lo = int(span[0], 16)
        hi = int(span[1], 16) if len(span) > 1 else lo
        out.append((lo, hi))
    out.sort()
    merged = []
    for lo, hi in out:
        if merged and merged[-1][1] + 1 >= lo:
            merged[-1] = (merged[-1][0], max(merged[-1][1], hi))
        else:
            merged.append((lo, hi))
    return merged


# --------------------------------------------------------------------------
# compression
# --------------------------------------------------------------------------

def case_ranges(mapping):
    """Compress {cp: mapped} into (lo, hi, stride, delta) runs.

    A run is a maximal set of code points lo, lo+stride, ... hi that all shift
    by the same delta. Stride 2 matters because Latin Extended and Cyrillic
    alternate upper/lower (U+0100 A-macron, U+0101 a-macron, U+0102 A-breve,
    ...), so the uppercase mappings sit on every other code point. Both stride
    candidates are tried and the longer run wins.
    """
    pairs = sorted(mapping.items())
    out = []
    i, n = 0, len(pairs)
    while i < n:
        cp, to = pairs[i]
        delta = to - cp
        best_end, best_stride = i, 1
        for stride in (1, 2):
            j = i
            while (
                j + 1 < n
                and pairs[j + 1][0] == pairs[j][0] + stride
                and pairs[j + 1][1] - pairs[j + 1][0] == delta
            ):
                j += 1
            if j > best_end:
                best_end, best_stride = j, stride
        out.append((cp, pairs[best_end][0], best_stride, delta))
        i = best_end + 1
    return out


# --------------------------------------------------------------------------
# output
# --------------------------------------------------------------------------

class Emitter:
    def __init__(self):
        self.parts = []

    def raw(self, text):
        self.parts.append(text)

    def blank(self):
        self.parts.append("")

    def comment(self, text):
        for line in text.strip("\n").split("\n"):
            self.parts.append(("// " + line).rstrip())

    def ints(self, name, values, per_line=8):
        if not values:
            sys.exit("refusing to emit empty table %s" % name)
        self.parts.append("const %s := [" % name)
        for i in range(0, len(values), per_line):
            row = ", ".join(str(v) for v in values[i:i + per_line])
            self.parts.append("    " + row)
        self.parts.append("]")

    def strings(self, name, values, per_line=6):
        if not values:
            sys.exit("refusing to emit empty table %s" % name)
        self.parts.append("const %s := [" % name)
        for i in range(0, len(values), per_line):
            row = ", ".join('"%s"' % v for v in values[i:i + per_line])
            self.parts.append("    " + row)
        self.parts.append("]")

    def text(self):
        return "\n".join(self.parts).rstrip() + "\n"


def build(cats, upper, lower, title, fold_simple, fold_full, spaces, alpha):
    e = Emitter()
    e.raw(BANNER)
    e.blank()
    e.comment(
        "GENERATED FILE - DO NOT EDIT.\n"
        "\n"
        "Produced by tools/python/gen_unicode_tables.py from the Unicode\n"
        "Character Database version %s. To change anything here, edit that\n"
        "script and re-run it; hand edits are lost on the next regeneration.\n"
        "\n"
        "The lookup functions that read these tables live in unicode.salam.\n"
        "Every table is sorted by its first column so all of them can be\n"
        "searched with the same binary search." % UCD_VERSION
    )
    e.blank()
    e.raw("package unicode")
    e.blank()

    e.comment(
        "The Unicode version these tables were generated from, as a string so\n"
        "a program can report it without the compiler folding it into a number."
    )
    e.raw('pub const Version := "%s"' % UCD_VERSION)
    e.blank()

    e.comment(
        "The longest full case fold in this version, in code points. A buffer\n"
        "sized for this can hold the fold of any single code point."
    )
    e.raw("pub const MaxFoldLen := %d" % max(len(v) for v in fold_full.values()))
    e.blank()

    # ---- category ids -----------------------------------------------------
    e.comment(
        "General category ids. Categories sharing a first letter are numbered\n"
        "consecutively so that the group predicates in unicode.salam reduce to\n"
        "a range check on the id. CatCn (unassigned) is last and is what\n"
        "Category returns for any code point not covered by _CAT_LO/_CAT_HI."
    )
    for name in CATEGORIES:
        e.raw("pub const Cat%s := %d" % (name, CAT_ID[name]))
    e.blank()

    e.comment("Two-letter category names, indexed by the ids above.")
    e.strings("_CAT_NAMES", list(CATEGORIES))
    e.blank()

    # ---- categories -------------------------------------------------------
    assigned = [(lo, hi, cat) for lo, hi, cat in cats if cat != "Cn"]
    e.comment(
        "Assigned code point ranges and their general category, sorted and\n"
        "non-overlapping. Unassigned code points are simply absent: %d ranges\n"
        "cover the whole of Unicode %s, which is far smaller than storing a\n"
        "category for each of the %d code points in the space." %
        (len(assigned), UCD_VERSION, MAX_CP)
    )
    e.ints("_CAT_LO", [r[0] for r in assigned])
    e.blank()
    e.ints("_CAT_HI", [r[1] for r in assigned])
    e.blank()
    e.ints("_CAT_ID", [CAT_ID[r[2]] for r in assigned])
    e.blank()

    # ---- case mappings ----------------------------------------------------
    for label, mapping, prefix, human in (
        ("uppercase", upper, "_UP", "simple uppercase"),
        ("lowercase", lower, "_LOW", "simple lowercase"),
        ("titlecase", title, "_TITLE", "simple titlecase"),
        ("fold", fold_simple, "_FOLD", "simple case folding"),
    ):
        rs = case_ranges(mapping)
        e.comment(
            "%s: %d mappings compressed into %d (lo, hi, stride, delta) runs.\n"
            "A code point cp in a run maps to cp + delta when\n"
            "(cp - lo) is a multiple of stride." % (human.capitalize(), len(mapping), len(rs))
        )
        e.ints("%s_LO" % prefix, [r[0] for r in rs])
        e.blank()
        e.ints("%s_HI" % prefix, [r[1] for r in rs])
        e.blank()
        e.ints("%s_STRIDE" % prefix, [r[2] for r in rs])
        e.blank()
        e.ints("%s_DELTA" % prefix, [r[3] for r in rs])
        e.blank()

    # ---- full folding -----------------------------------------------------
    full = sorted(fold_full.items())
    seq, offs, lens = [], [], []
    for _, cps in full:
        offs.append(len(seq))
        lens.append(len(cps))
        seq.extend(cps)
    e.comment(
        "Full case folding for the %d code points whose fold is longer than one\n"
        "code point - German sharp s folds to \"ss\", the ligatures fold to their\n"
        "component letters, and a handful of Greek and Armenian forms expand\n"
        "similarly. These are why case-insensitive comparison cannot be done\n"
        "byte for byte or even code point for code point: the two sides can\n"
        "differ in length.\n"
        "\n"
        "The expansions are code points rather than strings so that this\n"
        "package stays free of any string dependency - std/str imports\n"
        "std/unicode, so the arrow cannot point back. _FOLDF_CP is the sorted\n"
        "key column; _FOLDF_OFF and _FOLDF_LEN slice the expansion out of the\n"
        "flattened _FOLDF_SEQ." % len(full)
    )
    e.ints("_FOLDF_CP", [cp for cp, _ in full])
    e.blank()
    e.ints("_FOLDF_OFF", offs)
    e.blank()
    e.ints("_FOLDF_LEN", lens)
    e.blank()
    e.ints("_FOLDF_SEQ", seq)
    e.blank()

    # ---- properties -------------------------------------------------------
    e.comment(
        "The White_Space property from PropList.txt (%d ranges). This is the\n"
        "authoritative answer to \"is this a space\" and is deliberately not the\n"
        "same as \"category Z\": it adds the ASCII controls tab through carriage\n"
        "return and NEL, and it excludes ZERO WIDTH NON-JOINER and friends,\n"
        "which are format characters rather than spaces." % len(spaces)
    )
    e.ints("_WS_LO", [r[0] for r in spaces])
    e.blank()
    e.ints("_WS_HI", [r[1] for r in spaces])
    e.blank()

    e.comment(
        "The Alphabetic property from DerivedCoreProperties.txt (%d ranges).\n"
        "Broader than category L: it also takes in letter numbers, the marks\n"
        "that carry vowels in Indic and Arabic script, and other code points\n"
        "with Other_Alphabetic. This, not L alone, is what a caller asking\n"
        "\"is this a letter\" almost always means." % len(alpha)
    )
    e.ints("_ALPHA_LO", [r[0] for r in alpha])
    e.blank()
    e.ints("_ALPHA_HI", [r[1] for r in alpha])

    return e.text()


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--fetch", action="store_true",
                    help="download the UCD files into the cache before generating")
    ap.add_argument("--out", default=OUT, help="output path (default: %s)" % OUT)
    args = ap.parse_args()

    if args.fetch:
        fetch()

    cats, upper, lower, title = parse_unicode_data()
    fold_simple, fold_full = parse_case_folding()
    spaces = parse_prop("PropList.txt", "White_Space")
    alpha = parse_prop("DerivedCoreProperties.txt", "Alphabetic")

    text = build(cats, upper, lower, title, fold_simple, fold_full, spaces, alpha)

    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    with open(args.out, "w", encoding="utf-8") as f:
        f.write(text)

    sys.stderr.write(
        "wrote %s (UCD %s): %d category ranges, %d/%d/%d/%d case runs, "
        "%d full folds, %d space ranges, %d alphabetic ranges\n"
        % (
            os.path.relpath(args.out, ROOT), UCD_VERSION,
            len([c for c in cats if c[2] != "Cn"]),
            len(case_ranges(upper)), len(case_ranges(lower)),
            len(case_ranges(title)), len(case_ranges(fold_simple)),
            len(fold_full), len(spaces), len(alpha),
        )
    )


if __name__ == "__main__":
    main()
