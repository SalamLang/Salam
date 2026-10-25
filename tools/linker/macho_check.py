"""Checks a Mach-O executable written by the built-in linker.

sig <exe>              verify the ad-hoc code signature page hashes
cmp <ours> <ref>       compare disassembly with an ld64.lld reference after
                       taking out addresses, page numbers, local label
                       names and lld's GOT relaxations
"""

import difflib
import hashlib
import re
import struct
import subprocess
import sys


def code_signature(d):
    ncmds = struct.unpack_from("<I", d, 16)[0]
    off = 32
    cs = None
    for _ in range(ncmds):
        cmd, size = struct.unpack_from("<II", d, off)
        if cmd == 0x1D:
            cs = struct.unpack_from("<II", d, off + 8)
        off += size
    return cs


def check_sig(path):
    with open(path, "rb") as fh:
        d = fh.read()
    cs = code_signature(d)
    if cs is None:
        return "no LC_CODE_SIGNATURE"
    do = cs[0]
    if struct.unpack_from(">I", d, do)[0] != 0xFADE0CC0:
        return "bad SuperBlob magic"
    cd = do + struct.unpack_from(">I", d, do + 16)[0]
    f = struct.unpack_from(">IIIIIIIII", d, cd)
    if f[0] != 0xFADE0C02:
        return "bad CodeDirectory magic"
    hashes = cd + f[4]
    page = 1 << d[cd + 39]
    for i in range(f[7]):
        start = i * page
        stop = min(start + page, f[8])
        h0 = hashes + 32 * i
        h1 = h0 + 32
        if hashlib.sha256(d[start:stop]).digest() != d[h0:h1]:
            return f"page {i} hash mismatch"
    return ""


def norm(path):
    out = subprocess.run(
        [
            "llvm-objdump",
            "--macho",
            "-d",
            "--no-show-raw-insn",
            "--no-leading-addr",
            path,
        ],
        capture_output=True,
        text=True,
        check=True,
    ).stdout
    lines = []
    for ln in out.split("\n"):
        ln = re.sub(r"0x[0-9a-f]+", "", ln)
        ln = re.sub(r"#-?[0-9]+", "", ln)
        ln = re.sub(r"adrp\t(x\d+), -?\d+", r"adrp \1", ln)
        ln = ln.replace("__mh_execute_header", "")
        ln = re.sub(r"\b[lL]_[\w.$]+(?=\(%rip\))", "", ln)
        ln = re.sub(r", \[(x\d+), #\]", r", [\1]", ln)
        if (
            not ln
            or ln.startswith("/")
            or re.match(r"^(l_|ltmp)", ln)
            or ln.endswith(":")
            and ln.startswith(path)
        ):
            continue
        lines.append(ln)
    return lines


BENIGN = [
    r"adrp",
    r"ldr\tx\d+, \[x\d+\]$",
    r"add\tx\d+, x\d+, #$",
    r"movq\t\(%rip\), %r",
    r"leaq\t\(%rip\), %r",
    r"leaq\tl_\w+\(%rip\)",
    r"leaq\t-\(%rip\)",
    r":$",
]


def compare(ours, ref):
    bad = []
    for ln in difflib.unified_diff(norm(ours), norm(ref), lineterm="", n=0):
        changed = ln[:1] in "+-" and not ln.startswith(("+++", "---"))
        if changed and not any(re.search(b, ln) for b in BENIGN):
            bad.append(ln)
    return bad


def main():
    if len(sys.argv) >= 3 and sys.argv[1] == "sig":
        err = check_sig(sys.argv[2])
        if err:
            print(err)
            return 1
        return 0
    if len(sys.argv) >= 4 and sys.argv[1] == "cmp":
        bad = compare(sys.argv[2], sys.argv[3])
        for ln in bad[:5]:
            print(ln)
        return 1 if bad else 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main())
