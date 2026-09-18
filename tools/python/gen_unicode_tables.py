#!/usr/bin/env python3
"""Regenerate std/unicode/tables.salam from the Unicode Character Database.

std/unicode is a data package: every table in tables.salam is derived
mechanically from the UCD text files listed in FILES below, so the file is
generated rather than hand-maintained. Editing tables.salam directly is a
mistake - change this script and re-run it instead.

The inputs and what each one contributes:

    UnicodeData.txt              general category, simple upper/lower/title,
                                 canonical combining class, decompositions
    CaseFolding.txt              simple (C+S) and full (C+F) case folding
    PropList.txt                 White_Space
    DerivedCoreProperties.txt    Alphabetic
    DerivedNormalizationProps.txt composition exclusions
    auxiliary/GraphemeBreakProperty.txt  Grapheme_Cluster_Break
    emoji/emoji-data.txt         Extended_Pictographic

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

# This is a code generator: one pass reads the UCD and one function writes
# every table in tables.salam in the order the file declares them. That shape
# is the point - the emit order IS the output - so the size and argument-count
# checks below describe the job rather than a problem to refactor away. The
# remaining %-formats are multi-line wrapped comment text, where an f-string
# would mean prefixing every continuation fragment.
# pylint: disable=too-many-lines,too-many-arguments,too-many-positional-arguments
# pylint: disable=too-many-locals,too-many-statements,consider-using-f-string

import argparse
import os
import sys
import urllib.request

UCD_VERSION = "16.0.0"
UCD_BASE = f"https://www.unicode.org/Public/{UCD_VERSION}/ucd"
# Remote path relative to UCD_BASE -> local filename in the cache. Two of these
# live in subdirectories of the UCD, so the mapping cannot be implied.
FILES = {
    "UnicodeData.txt": "UnicodeData.txt",
    "CaseFolding.txt": "CaseFolding.txt",
    "PropList.txt": "PropList.txt",
    "DerivedCoreProperties.txt": "DerivedCoreProperties.txt",
    "DerivedNormalizationProps.txt": "DerivedNormalizationProps.txt",
    "auxiliary/GraphemeBreakProperty.txt": "GraphemeBreakProperty.txt",
    "emoji/emoji-data.txt": "emoji-data.txt",
    "extracted/DerivedBidiClass.txt": "DerivedBidiClass.txt",
    "BidiBrackets.txt": "BidiBrackets.txt",
    "Scripts.txt": "Scripts.txt",
}

# Hangul is composed and decomposed by arithmetic rather than by table - the
# 11172 syllables are a regular product of 19 leading jamo, 21 vowels and 28
# trailing jamo. Tabling them would add ~11000 decomposition entries and ~800
# grapheme-break ranges for no benefit, so the runtime special-cases this range
# instead. Constants are from UAX #15 section 3.12.
HANGUL_SBASE = 0xAC00
HANGUL_LBASE = 0x1100
HANGUL_VBASE = 0x1161
HANGUL_TBASE = 0x11A7
HANGUL_LCOUNT = 19
HANGUL_VCOUNT = 21
HANGUL_TCOUNT = 28
HANGUL_NCOUNT = HANGUL_VCOUNT * HANGUL_TCOUNT
HANGUL_SCOUNT = HANGUL_LCOUNT * HANGUL_NCOUNT

# Grapheme_Cluster_Break classes, in the order UAX #29 lists them. Other is 0
# so that an unlisted code point falls into it by default. LV and LVT are
# absent: they are exactly the Hangul syllable block and are derived
# arithmetically, as above.
GRAPHEME_CLASSES = (
    "Other",
    "CR",
    "LF",
    "Control",
    "Extend",
    "ZWJ",
    "Regional_Indicator",
    "Prepend",
    "SpacingMark",
    "L",
    "V",
    "T",
    "LV",
    "LVT",
    "Extended_Pictographic",
)
GRAPHEME_ID = {name: i for i, name in enumerate(GRAPHEME_CLASSES)}

# Indic_Conjunct_Break values. None is 0 and is the default for anything the
# property does not list.
INCB_CLASSES = ("None", "Consonant", "Extend", "Linker")
INCB_ID = {name: i for i, name in enumerate(INCB_CLASSES)}

# Bidi_Class values, grouped so that the algorithm's repeated "is this one of
# the explicit formatting codes" tests become range checks. L is 0 and is the
# implicit value for everything the table does not list.
BIDI_CLASSES = (
    "L",
    "R",
    "AL",  # strong
    "EN",
    "ES",
    "ET",
    "AN",
    "CS",
    "NSM",
    "BN",  # weak
    "B",
    "S",
    "WS",
    "ON",  # neutral
    "LRE",
    "RLE",
    "LRO",
    "RLO",
    "PDF",  # explicit embedding/override
    "LRI",
    "RLI",
    "FSI",
    "PDI",  # explicit isolate
)
BIDI_ID = {name: i for i, name in enumerate(BIDI_CLASSES)}
BIDI_LONG = {
    "Left_To_Right": "L",
    "Right_To_Left": "R",
    "Arabic_Letter": "AL",
    "European_Number": "EN",
    "European_Separator": "ES",
    "European_Terminator": "ET",
    "Arabic_Number": "AN",
    "Common_Separator": "CS",
    "Nonspacing_Mark": "NSM",
    "Boundary_Neutral": "BN",
    "Paragraph_Separator": "B",
    "Segment_Separator": "S",
    "White_Space": "WS",
    "Other_Neutral": "ON",
}

# Bidi_Paired_Bracket_Type, for rule N0.
PBT_CLASSES = ("None", "Open", "Close")
PBT_ID = {name: i for i, name in enumerate(PBT_CLASSES)}

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
    "Lu",
    "Ll",
    "Lt",
    "Lm",
    "Lo",
    "Mn",
    "Mc",
    "Me",
    "Nd",
    "Nl",
    "No",
    "Pc",
    "Pd",
    "Ps",
    "Pe",
    "Pi",
    "Pf",
    "Po",
    "Sm",
    "Sc",
    "Sk",
    "So",
    "Zs",
    "Zl",
    "Zp",
    "Cc",
    "Cf",
    "Cs",
    "Co",
    "Cn",
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
    for remote, local in FILES.items():
        url = f"{UCD_BASE}/{remote}"
        sys.stderr.write(f"fetching {url}\n")
        with urllib.request.urlopen(url) as r:
            data = r.read()
        with open(ucd_path(local), "wb") as f:
            f.write(data)


def read_lines(name):
    path = ucd_path(name)
    if not os.path.exists(path):
        sys.exit(
            f"missing {path}\nRun with --fetch to download the UCD {UCD_VERSION} files."
        )
    with open(path, encoding="utf-8") as f:
        for line in f:
            line = line.split("#", 1)[0].strip()
            if line:
                yield line


def parse_unicode_data():
    """-> (categories, upper, lower, title, ccc, canon, compat)

    categories is a {cp: category} style range list built as (lo, hi, cat)
    triples; the case maps are {cp: mapped_cp} dicts of the simple mappings;
    ccc is {cp: canonical_combining_class} for the non-zero classes; canon and
    compat are the one-step decomposition mappings, {cp: [cps]}.

    UnicodeData.txt lists large uniform blocks (CJK, Hangul, the private use
    areas) as a First/Last pair of lines rather than one line per code point,
    so those have to be expanded into a range instead of read literally.
    """
    ranges = []
    upper, lower, title = {}, {}, {}
    ccc, canon, compat = {}, {}, {}
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
                sys.exit(f"unpaired range terminator at U+{cp:04X}")
            ranges.append((pending_first[0], cp, cat))
            pending_first = None
            continue

        ranges.append((cp, cp, cat))
        if f[3] != "0":
            ccc[cp] = int(f[3])
        if f[5]:
            # A decomposition tagged <compat>, <font>, <isolated> and so on is
            # a compatibility mapping; an untagged one is canonical. Only the
            # canonical mappings take part in NFC/NFD.
            if f[5].startswith("<"):
                compat[cp] = [int(x, 16) for x in f[5].split(">", 1)[1].split()]
            else:
                canon[cp] = [int(x, 16) for x in f[5].split()]
        if f[12]:
            upper[cp] = int(f[12], 16)
        if f[13]:
            lower[cp] = int(f[13], 16)
        if f[14]:
            title[cp] = int(f[14], 16)

    if pending_first is not None:
        sys.exit(f"unterminated range starting at U+{pending_first[0]:04X}")

    return merge_cat_ranges(ranges), upper, lower, title, ccc, canon, compat


def expand_decompositions(canon, compat):
    """-> (full_canon, full_compat)

    Applies each decomposition to its own result until nothing decomposes
    further, so the emitted tables hold fully decomposed forms. Doing the
    recursion here means the runtime never has to: one table lookup per code
    point produces the final answer, and a bug in the fixpoint shows up when
    this script runs rather than deep inside a normalization loop.

    Canonical decomposition never mixes in a compatibility mapping, so the two
    fixpoints are computed over different tables: full_canon closes over canon
    alone, full_compat over both (a compatibility decomposition may expose a
    canonical one and vice versa).
    """

    def fixpoint(cp, table, depth=0):
        if depth > 32:
            sys.exit(f"decomposition of U+{cp:04X} does not terminate")
        d = table.get(cp)
        if d is None:
            return [cp]
        out = []
        for c in d:
            out.extend(fixpoint(c, table, depth + 1))
        return out

    both = dict(canon)
    both.update(compat)
    full_canon = {cp: fixpoint(cp, canon) for cp in canon}
    full_compat = {cp: fixpoint(cp, both) for cp in both}
    return full_canon, full_compat


def composition_pairs(canon, exclusions):
    """-> sorted [(starter, combining, composed)]

    The NFC composition table is the canonical decomposition table read
    backwards, restricted to the two-code-point mappings and with the
    composition exclusions removed. A code point is excluded when composing it
    back would undo a distinction the source made deliberately - the Hebrew
    and Arabic ligature forms, the precomposed Cyrillic that Unicode later
    decided was a mistake, and the singleton decompositions.
    """
    out = []
    for cp, d in canon.items():
        if len(d) == 2 and cp not in exclusions:
            out.append((d[0], d[1], cp))
    out.sort()
    return out


def parse_scripts():
    """-> (script_names, sorted merged [(lo, hi, script_id)])

    The Script property. Unknown is 0 and is the default, so only the assigned
    ranges are emitted.

    This is what lets a question like "is this Arabic script" be asked properly
    instead of by testing block ranges. Arabic script is spread over the main
    block, the supplement, the extended blocks and both presentation-form
    blocks, and those blocks also hold code points that are not Arabic script -
    so a block test is wrong in both directions.
    """
    names = ["Unknown"]
    index = {"Unknown": 0}
    ranges = []
    for line in read_lines("Scripts.txt"):
        f = [p.strip() for p in line.split(";")]
        if len(f) < 2:
            continue
        if f[1] not in index:
            index[f[1]] = len(names)
            names.append(f[1])
        span = f[0].split("..")
        lo = int(span[0], 16)
        hi = int(span[1], 16) if len(span) > 1 else lo
        ranges.append((lo, hi, index[f[1]]))
    ranges.sort()
    merged = []
    for lo, hi, sid in ranges:
        if merged and merged[-1][2] == sid and merged[-1][1] + 1 >= lo:
            merged[-1] = (merged[-1][0], max(merged[-1][1], hi), sid)
        else:
            merged.append((lo, hi, sid))
    return names, merged


def parse_bidi_class():
    """-> sorted, merged [(lo, hi, class_id)] for Bidi_Class, excluding L.

    Bidi_Class cannot be read as "listed value, else default L". Unassigned
    code points inside the Hebrew, Arabic, Thaana and related blocks default to
    R or AL rather than L, and DerivedBidiClass.txt states those defaults only
    in @missing comment lines - which the normal reader strips. Getting this
    wrong makes unassigned code points in an Arabic block behave as
    left-to-right, which shows up as text that reorders correctly today and
    incorrectly the next time Unicode assigns a character in that block.

    So the map is built in layers: the file-wide default, then each block
    default, then the explicit data lines on top. L is the implicit answer for
    anything not emitted.
    """
    default_ranges = []
    with open(ucd_path("DerivedBidiClass.txt"), encoding="utf-8") as f:
        for line in f:
            if "@missing:" not in line:
                continue
            spec = line.split("@missing:", 1)[1].strip()
            span, name = [p.strip() for p in spec.split(";")]
            if name not in BIDI_LONG:
                sys.exit(f"unknown Bidi_Class default {name!r}")
            lo, hi = span.split("..")
            default_ranges.append((int(lo, 16), int(hi, 16), BIDI_LONG[name]))
    if not default_ranges:
        sys.exit("DerivedBidiClass.txt has no @missing defaults")

    bc = {}
    for lo, hi, name in default_ranges:
        for cp in range(lo, hi + 1):
            bc[cp] = name
    for line in read_lines("DerivedBidiClass.txt"):
        f = [p.strip() for p in line.split(";")]
        if len(f) < 2:
            continue
        if f[1] not in BIDI_ID:
            sys.exit(f"unknown Bidi_Class {f[1]!r}")
        span = f[0].split("..")
        lo = int(span[0], 16)
        hi = int(span[1], 16) if len(span) > 1 else lo
        for cp in range(lo, hi + 1):
            bc[cp] = f[1]

    non_default = {cp: v for cp, v in bc.items() if v != "L"}
    return [(lo, hi, BIDI_ID[v]) for lo, hi, v in value_ranges(non_default)]


def parse_brackets():
    """-> sorted [(cp, paired_cp, type_id)] from BidiBrackets.txt.

    Rule N0 matches a bracket against its pair inside one isolating run
    sequence, so both halves of the mapping are needed.
    """
    out = []
    for line in read_lines("BidiBrackets.txt"):
        f = [p.strip() for p in line.split(";")]
        if len(f) < 3:
            continue
        kind = {"o": PBT_ID["Open"], "c": PBT_ID["Close"]}.get(f[2])
        if kind is None:
            sys.exit(f"unknown Bidi_Paired_Bracket_Type {f[2]!r}")
        out.append((int(f[0], 16), int(f[1], 16), kind))
    out.sort()
    return out


def parse_incb():
    """-> sorted, merged [(lo, hi, class_id)] for Indic_Conjunct_Break.

    A three-valued property (Consonant, Extend, Linker) written in
    DerivedCoreProperties.txt as `range ; InCB ; value`, so it needs its own
    parser rather than parse_prop's two-field form. It exists for one rule:
    UAX #29's GB9c, which holds a Devanagari conjunct such as क्त together as a
    single grapheme cluster instead of breaking it at the virama.
    """
    ranges = []
    for line in read_lines("DerivedCoreProperties.txt"):
        f = [p.strip() for p in line.split(";")]
        if len(f) < 3 or f[1] != "InCB":
            continue
        if f[2] not in INCB_ID:
            sys.exit(f"unknown Indic_Conjunct_Break value {f[2]!r}")
        span = f[0].split("..")
        lo = int(span[0], 16)
        hi = int(span[1], 16) if len(span) > 1 else lo
        ranges.append((lo, hi, INCB_ID[f[2]]))

    ranges.sort()
    merged = []
    for lo, hi, cid in ranges:
        if merged and merged[-1][2] == cid and merged[-1][1] + 1 >= lo:
            merged[-1] = (merged[-1][0], max(merged[-1][1], hi), cid)
        else:
            merged.append((lo, hi, cid))
    return merged


def parse_grapheme_break():
    """-> sorted, merged [(lo, hi, class_id)] for Grapheme_Cluster_Break.

    Extended_Pictographic comes from a different file (emoji-data.txt) but is
    needed by the same rule set - UAX #29's GB11 keeps an emoji joined to a
    following emoji by ZWJ in one cluster - so it is folded in here as another
    class. The Hangul LV and LVT classes are dropped: they cover exactly the
    syllable block and the runtime derives them arithmetically.
    """
    ranges = []
    for line in read_lines("GraphemeBreakProperty.txt"):
        f = [p.strip() for p in line.split(";")]
        if len(f) < 2 or f[1] in ("LV", "LVT"):
            continue
        if f[1] not in GRAPHEME_ID:
            sys.exit(f"unknown Grapheme_Cluster_Break class {f[1]!r}")
        span = f[0].split("..")
        lo = int(span[0], 16)
        hi = int(span[1], 16) if len(span) > 1 else lo
        ranges.append((lo, hi, GRAPHEME_ID[f[1]]))

    for lo, hi in parse_prop("emoji-data.txt", "Extended_Pictographic"):
        ranges.append((lo, hi, GRAPHEME_ID["Extended_Pictographic"]))

    ranges.sort()
    for a, b in zip(ranges, ranges[1:]):
        if b[0] <= a[1]:
            sys.exit(f"overlapping grapheme break ranges at U+{b[0]:04X}")

    merged = []
    for lo, hi, cid in ranges:
        if merged and merged[-1][2] == cid and merged[-1][1] + 1 == lo:
            merged[-1] = (merged[-1][0], hi, cid)
        else:
            merged.append((lo, hi, cid))
    return merged


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
                sys.exit(f"non-simple mapping for status {status} at U+{cp:04X}")
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


def value_ranges(mapping):
    """Compress {cp: value} into maximal (lo, hi, value) runs of consecutive
    code points sharing a value."""
    items = sorted(mapping.items())
    out = []
    i, n = 0, len(items)
    while i < n:
        cp, v = items[i]
        j = i
        while j + 1 < n and items[j + 1][0] == items[j][0] + 1 and items[j + 1][1] == v:
            j += 1
        out.append((cp, items[j][0], v))
        i = j + 1
    return out


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
    """Accumulates the lines of tables.salam and joins them at the end."""

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
            sys.exit(f"refusing to emit empty table {name}")
        self.parts.append(f"const {name} := [")
        for i in range(0, len(values), per_line):
            row = ", ".join(str(v) for v in values[i : i + per_line])
            self.parts.append("    " + row)
        self.parts.append("]")

    def strings(self, name, values, per_line=6):
        if not values:
            sys.exit(f"refusing to emit empty table {name}")
        self.parts.append(f"const {name} := [")
        for i in range(0, len(values), per_line):
            row = ", ".join(f'"{v}"' for v in values[i : i + per_line])
            self.parts.append("    " + row)
        self.parts.append("]")

    def text(self):
        return "\n".join(self.parts).rstrip() + "\n"


def build(
    cats,
    upper,
    lower,
    title,
    fold_simple,
    fold_full,
    spaces,
    alpha,
    ccc,
    full_canon,
    full_compat,
    comp,
    gcb,
    incb,
    bidi,
    brackets,
    script_names,
    scripts,
):
    e = Emitter()
    e.raw(BANNER)
    e.blank()
    e.comment(
        "GENERATED FILE - DO NOT EDIT.\n"
        "\n"
        "Produced by tools/python/gen_unicode_tables.py from the Unicode\n"
        f"Character Database version {UCD_VERSION}. To change anything here, edit that\n"
        "script and re-run it; hand edits are lost on the next regeneration.\n"
        "\n"
        "The lookup functions that read these tables live in unicode.salam.\n"
        "Every table is sorted by its first column so all of them can be\n"
        "searched with the same binary search."
    )
    e.blank()
    e.raw("package unicode")
    e.blank()

    e.comment(
        "The Unicode version these tables were generated from, as a string so\n"
        "a program can report it without the compiler folding it into a number."
    )
    e.raw(f'pub const Version := "{UCD_VERSION}"')
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
        "category for each of the %d code points in the space."
        % (len(assigned), UCD_VERSION, MAX_CP)
    )
    e.ints("_CAT_LO", [r[0] for r in assigned])
    e.blank()
    e.ints("_CAT_HI", [r[1] for r in assigned])
    e.blank()
    e.ints("_CAT_ID", [CAT_ID[r[2]] for r in assigned])
    e.blank()

    # ---- case mappings ----------------------------------------------------
    for mapping, prefix, human in (
        (upper, "_UP", "simple uppercase"),
        (lower, "_LOW", "simple lowercase"),
        (title, "_TITLE", "simple titlecase"),
        (fold_simple, "_FOLD", "simple case folding"),
    ):
        rs = case_ranges(mapping)
        e.comment(
            "%s: %d mappings compressed into %d (lo, hi, stride, delta) runs.\n"
            "A code point cp in a run maps to cp + delta when\n"
            "(cp - lo) is a multiple of stride."
            % (human.capitalize(), len(mapping), len(rs))
        )
        e.ints(f"{prefix}_LO", [r[0] for r in rs])
        e.blank()
        e.ints(f"{prefix}_HI", [r[1] for r in rs])
        e.blank()
        e.ints(f"{prefix}_STRIDE", [r[2] for r in rs])
        e.blank()
        e.ints(f"{prefix}_DELTA", [r[3] for r in rs])
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
        'code point - German sharp s folds to "ss", the ligatures fold to their\n'
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
        'authoritative answer to "is this a space" and is deliberately not the\n'
        'same as "category Z": it adds the ASCII controls tab through carriage\n'
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
        '"is this a letter" almost always means.' % len(alpha)
    )
    e.ints("_ALPHA_LO", [r[0] for r in alpha])
    e.blank()
    e.ints("_ALPHA_HI", [r[1] for r in alpha])
    e.blank()

    # ---- combining class --------------------------------------------------
    ccc_runs = value_ranges(ccc)
    e.comment(
        "Canonical combining class, as %d (lo, hi, class) runs covering the %d\n"
        "code points whose class is not zero. Everything absent is class 0.\n"
        "\n"
        "This is the key to canonical ordering: when several marks follow one\n"
        "base character, normalization sorts them by this number, so that a\n"
        "Persian word written with its harakat in one order compares equal to\n"
        "the same word with them typed in another. Marks of class 0, and any\n"
        "two marks of equal class, are never reordered relative to each other."
        % (len(ccc_runs), len(ccc))
    )
    e.ints("_CCC_LO", [r[0] for r in ccc_runs])
    e.blank()
    e.ints("_CCC_HI", [r[1] for r in ccc_runs])
    e.blank()
    e.ints("_CCC_VAL", [r[2] for r in ccc_runs])
    e.blank()

    # ---- decompositions ---------------------------------------------------
    for prefix, table, human, extra in (
        (
            "_NFD",
            full_canon,
            "Canonical",
            "These are the mappings NFD and NFC use. Each is stored fully\n"
            "expanded, so no entry here decomposes any further.",
        ),
        (
            "_NFKD",
            full_compat,
            "Compatibility",
            "These add the mappings NFKD and NFKC use on top of the canonical\n"
            "ones. For Arabic and Persian this is the table that matters most:\n"
            "the presentation forms in U+FB50..U+FDFF and U+FE70..U+FEFF - the\n"
            "isolated, initial, medial and final shapes that some sources store\n"
            "literally instead of letting the renderer choose - decompose back to\n"
            "the plain letters here, and nowhere else.",
        ),
    ):
        items = sorted(table.items())
        seq, offs, lens = [], [], []
        for _, cps in items:
            offs.append(len(seq))
            lens.append(len(cps))
            seq.extend(cps)
        e.comment(
            "%s decomposition: %d code points expanding to %d in total, at\n"
            "most %d each.\n"
            "\n"
            "%s\n"
            "\n"
            "Hangul is not in this table. Its %d syllables decompose by\n"
            "arithmetic instead - see the Hangul constants in unicode.salam."
            % (human, len(items), len(seq), max(lens), extra, HANGUL_SCOUNT)
        )
        e.ints(f"{prefix}_CP", [cp for cp, _ in items])
        e.blank()
        e.ints(f"{prefix}_OFF", offs)
        e.blank()
        e.ints(f"{prefix}_LEN", lens)
        e.blank()
        e.ints(f"{prefix}_SEQ", seq)
        e.blank()

    # ---- composition ------------------------------------------------------
    e.comment(
        "Canonical composition: %d (starter, combining) pairs that recombine\n"
        "into a single code point. This is the decomposition table read\n"
        "backwards, minus the composition exclusions - the pairs Unicode says\n"
        "must stay apart even though they decomposed.\n"
        "\n"
        "Sorted by starter and then by the combining mark, so a lookup binary\n"
        "searches _COMP_A for the starter and then scans that starter's short\n"
        "run for the mark." % len(comp)
    )
    e.ints("_COMP_A", [c[0] for c in comp])
    e.blank()
    e.ints("_COMP_B", [c[1] for c in comp])
    e.blank()
    e.ints("_COMP_CP", [c[2] for c in comp])
    e.blank()

    # ---- grapheme cluster break -------------------------------------------
    e.comment("Grapheme_Cluster_Break class ids, used by the segmenter in str.")
    for name in GRAPHEME_CLASSES:
        e.raw("pub const Gcb%s := %d" % (name.replace("_", ""), GRAPHEME_ID[name]))
    e.blank()

    e.comment(
        "Grapheme_Cluster_Break, as %d (lo, hi, class) ranges. Code points not\n"
        "listed are GcbOther. The Hangul LV and LVT classes are absent and are\n"
        "derived arithmetically, which is why this is ~800 ranges smaller than\n"
        "GraphemeBreakProperty.txt.\n"
        "\n"
        "This is what lets text be counted and cut by what a reader sees as\n"
        "one character rather than by code point: an Arabic letter followed by\n"
        "two harakat is three code points and one grapheme cluster." % len(gcb)
    )
    e.ints("_GCB_LO", [r[0] for r in gcb])
    e.blank()
    e.ints("_GCB_HI", [r[1] for r in gcb])
    e.blank()
    e.ints("_GCB_ID", [r[2] for r in gcb])
    e.blank()

    # ---- indic conjunct break ---------------------------------------------
    e.comment("Indic_Conjunct_Break class ids, used by GB9c in the segmenter.")
    for name in INCB_CLASSES:
        e.raw("pub const Incb%s := %d" % (name, INCB_ID[name]))
    e.blank()

    e.comment(
        "Indic_Conjunct_Break, as %d (lo, hi, class) ranges; anything absent is\n"
        "IncbNone. This drives one rule, GB9c, which keeps a consonant joined\n"
        "to a following consonant by a virama in the same grapheme cluster -\n"
        "so Devanagari क्त counts as one character rather than three." % len(incb)
    )
    e.ints("_INCB_LO", [r[0] for r in incb])
    e.blank()
    e.ints("_INCB_HI", [r[1] for r in incb])
    e.blank()
    e.ints("_INCB_ID", [r[2] for r in incb])
    e.blank()

    # ---- bidi -------------------------------------------------------------
    e.comment(
        "Bidi_Class ids. Grouped by kind so the bidirectional algorithm's\n"
        'repeated "is this an explicit formatting code" tests are range\n'
        "checks: the embedding and override codes are BcLRE..BcPDF and the\n"
        "isolate codes are BcLRI..BcPDI."
    )
    for name in BIDI_CLASSES:
        e.raw("pub const Bc%s := %d" % (name, BIDI_ID[name]))
    e.blank()

    e.comment(
        "Bidi_Class, as %d (lo, hi, class) ranges. Anything absent is BcL.\n"
        "\n"
        "The absent-means-L rule holds only because the block defaults are\n"
        "baked in here: unassigned code points in the Hebrew and Arabic blocks\n"
        "are R and AL respectively, and they appear as explicit ranges below\n"
        "even though no character is assigned to them yet." % len(bidi)
    )
    e.ints("_BIDI_LO", [r[0] for r in bidi])
    e.blank()
    e.ints("_BIDI_HI", [r[1] for r in bidi])
    e.blank()
    e.ints("_BIDI_ID", [r[2] for r in bidi])
    e.blank()

    e.comment("Bidi_Paired_Bracket_Type ids, for rule N0.")
    for name in PBT_CLASSES:
        e.raw("pub const Pbt%s := %d" % (name, PBT_ID[name]))
    e.blank()

    e.comment(
        "Bidi_Paired_Bracket: the %d bracket code points, each with the code\n"
        "point it pairs with and whether it opens or closes. Rule N0 uses this\n"
        "to give both halves of a bracket pair the same direction, so that\n"
        '"(سلام)" does not come out with its parentheses facing the wrong way.'
        % len(brackets)
    )
    e.ints("_BRACKET_CP", [b[0] for b in brackets])
    e.blank()
    e.ints("_BRACKET_PAIR", [b[1] for b in brackets])
    e.blank()
    e.ints("_BRACKET_TYPE", [b[2] for b in brackets])
    e.blank()

    # ---- scripts ----------------------------------------------------------
    e.comment(
        "Script ids. ScriptUnknown is 0 and is what an unassigned code point\n"
        "answers. Only the scripts this version defines are listed."
    )
    for i, name in enumerate(script_names):
        e.raw("pub const Script%s := %d" % (name.replace("_", ""), i))
    e.blank()

    e.comment("Script names, indexed by the ids above.")
    e.strings("_SCRIPT_NAMES", script_names, per_line=4)
    e.blank()

    e.comment(
        "The Script property, as %d (lo, hi, script) ranges. Anything absent is\n"
        "ScriptUnknown." % len(scripts)
    )
    e.ints("_SCRIPT_LO", [r[0] for r in scripts])
    e.blank()
    e.ints("_SCRIPT_HI", [r[1] for r in scripts])
    e.blank()
    e.ints("_SCRIPT_ID", [r[2] for r in scripts])

    return e.text()


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--fetch",
        action="store_true",
        help="download the UCD files into the cache before generating",
    )
    ap.add_argument("--out", default=OUT, help=f"output path (default: {OUT})")
    args = ap.parse_args()

    if args.fetch:
        fetch()

    cats, upper, lower, title, ccc, canon, compat = parse_unicode_data()
    fold_simple, fold_full = parse_case_folding()
    spaces = parse_prop("PropList.txt", "White_Space")
    alpha = parse_prop("DerivedCoreProperties.txt", "Alphabetic")
    exclusions = set()
    for lo, hi in parse_prop(
        "DerivedNormalizationProps.txt", "Full_Composition_Exclusion"
    ):
        exclusions.update(range(lo, hi + 1))
    full_canon, full_compat = expand_decompositions(canon, compat)
    comp = composition_pairs(canon, exclusions)
    gcb = parse_grapheme_break()
    incb = parse_incb()
    bidi = parse_bidi_class()
    brackets = parse_brackets()
    script_names, scripts = parse_scripts()

    text = build(
        cats,
        upper,
        lower,
        title,
        fold_simple,
        fold_full,
        spaces,
        alpha,
        ccc,
        full_canon,
        full_compat,
        comp,
        gcb,
        incb,
        bidi,
        brackets,
        script_names,
        scripts,
    )

    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    with open(args.out, "w", encoding="utf-8") as f:
        f.write(text)

    sys.stderr.write(
        "wrote %s (UCD %s):\n"
        "  %d category ranges, %d/%d/%d/%d case runs, %d full folds\n"
        "  %d space ranges, %d alphabetic ranges, %d combining-class runs\n"
        "  %d canonical and %d compatibility decompositions\n"
        "  %d composition pairs, %d grapheme-break and %d conjunct-break ranges\n"
        "  %d bidi-class ranges, %d bracket pairs, %d script ranges\n"
        % (
            os.path.relpath(args.out, ROOT),
            UCD_VERSION,
            len([c for c in cats if c[2] != "Cn"]),
            len(case_ranges(upper)),
            len(case_ranges(lower)),
            len(case_ranges(title)),
            len(case_ranges(fold_simple)),
            len(fold_full),
            len(spaces),
            len(alpha),
            len(value_ranges(ccc)),
            len(full_canon),
            len(full_compat),
            len(comp),
            len(gcb),
            len(incb),
            len(bidi),
            len(brackets),
            len(scripts),
        )
    )


if __name__ == "__main__":
    main()
