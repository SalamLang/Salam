#!/usr/bin/env python3
"""abicheck - flag LLVM calls whose types disagree with the declaration.

LLVM accepts a call whose argument or return types differ from the callee's
declaration WITHOUT complaint, and lowers it using the call site's types. The
result is a silent ABI mismatch: on big-endian ARM an i64 passed to a function
declared taking i32 arrives as the high half, i.e. 0 for any small value.

This found four such bugs in the LLVM backend (write/malloc/closure-env/strtol),
present in 25 of 25 test programs on every 32-bit target. Run it over emitted
IR for a non-x86_64 target before trusting a backend change:

    salam llvm prog.salam --output=p.ll --target=i686-unknown-linux-gnu
    python tools/abicheck.py p.ll

Exits non-zero when it finds a mismatch. Variadic callees are skipped: C
argument promotion makes the comparison meaningless there.
"""

import re
import sys

DECL_RE = re.compile(
    r"^declare[^@\n]*?([a-zA-Z0-9_.]+|\{[^}]*\})\s+@([A-Za-z0-9_.$]+)\(([^)]*)\)",
    re.M,
)
CALL_RE = re.compile(r"=\s*call\s+([a-zA-Z0-9_.]+)\s+@([A-Za-z0-9_.$]+)\(([^)]*)\)")
VOID_CALL_RE = re.compile(
    r"^\s*call\s+([a-zA-Z0-9_.]+)\s+@([A-Za-z0-9_.$]+)\(([^)]*)\)", re.M
)


def _scan_decls(txt):
    decls = {}
    for m in DECL_RE.finditer(txt):
        ret, name, params = m.group(1), m.group(2), m.group(3)
        if "..." in params:
            continue  # variadic: C promotion rules apply
        ps = [p.strip().split(" ")[0] for p in params.split(",") if p.strip()]
        decls[name] = (ret, ps)
    return decls


def _mismatches(pattern, txt, decls):
    bad = []
    for m in pattern.finditer(txt):
        ret, name, args = m.group(1), m.group(2), m.group(3)
        if name not in decls:
            continue
        dret, dps = decls[name]
        at = [a.strip().split(" ")[0] for a in args.split(",") if a.strip()]
        if ret != dret:
            bad.append((name, "return", dret, ret))
            continue
        if len(at) != len(dps):
            continue
        for i, (x, y) in enumerate(zip(dps, at)):
            if x != y:
                bad.append((name, f"arg{i}", x, y))
    return bad


def check(path):
    with open(path, encoding="utf-8", errors="replace") as f:
        txt = f.read()
    decls = _scan_decls(txt)
    bad = _mismatches(CALL_RE, txt, decls)
    bad += _mismatches(VOID_CALL_RE, txt, decls)
    return bad


def main():
    bad = check(sys.argv[1])
    seen = set()
    for n, w, d, c in bad:
        k = (n, w, d, c)
        if k in seen:
            continue
        seen.add(k)
        print(f"  {n:<14} {w:<7} declared {d:<6} called {c}")
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
