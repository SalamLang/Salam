#!/usr/bin/env python3
"""Load an x86_64 Mach-O dylib on x86_64 Linux, run its initialisers and call
its exported functions.

Each call is RET:NAME:ARGS:VALUES, where RET and every letter of ARGS is one of
i (int), d (double), s (string) or v (void, RET only); VALUES is a comma
separated list. The result of every non-void call is printed on its own line,
then the process exits through libc so the library's atexit handlers run.

    python3 -I tools/linker/machodylib.py libm.dylib i:_salam_add:ii:40,2
"""

import ctypes
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import machorun  # noqa: E402

CTYPE = {"i": ctypes.c_int, "d": ctypes.c_double, "s": ctypes.c_char_p}


def sections(data):
    ncmds = struct.unpack_from("<I", data, 16)[0]
    off = 32
    out = []
    for _ in range(ncmds):
        cmd, csize = struct.unpack_from("<II", data, off)
        if cmd == 0x19:
            nsect = struct.unpack_from("<I", data, off + 64)[0]
            for k in range(nsect):
                s = off + 72 + 80 * k
                n0 = s + 16
                n1 = s + 32
                seg = bytes(data[n0:n1]).rstrip(b"\0").decode()
                s0 = s + 16
                name = bytes(data[s:s0]).rstrip(b"\0").decode()
                addr, size = struct.unpack_from("<QQ", data, s + 32)
                out.append((seg, name, addr, size))
        off += csize
    return out


def exports(data, dyld):
    start, size = dyld[8], dyld[9]
    found = {}

    def walk(node, prefix):
        p = start + node
        tsize, p = machorun.uleb(data, p)
        if tsize:
            q = p
            _, q = machorun.uleb(data, q)
            addr, _ = machorun.uleb(data, q)
            found[prefix] = addr
        p += tsize
        n = data[p]
        p += 1
        for _ in range(n):
            q = data.index(b"\0", p)
            edge = data[p:q].decode()
            child, p = machorun.uleb(data, q + 1)
            walk(child, prefix + edge)

    if size:
        walk(0, "")
    return found


def main():
    with open(sys.argv[1], "rb") as f:
        data = f.read()
    if struct.unpack_from("<I", data, 12)[0] != 6:
        raise SystemExit("machodylib: not a dylib")
    segs, dyld, _ = machorun.parse(data)
    img = machorun.Image(data, segs)
    machorun.link(img, data, dyld)
    init = ctypes.CFUNCTYPE(None)
    for seg, name, addr, size in sections(data):
        if name == "__mod_init_func":
            for k in range(size // 8):
                init(img.get64(addr + 8 * k))()
    table = exports(data, dyld)
    for spec in sys.argv[2:]:
        ret, name, args, values = spec.split(":", 3)
        if name not in table:
            raise SystemExit("machodylib: " + name + " is not exported")
        restype = None if ret == "v" else CTYPE[ret]
        fn = ctypes.CFUNCTYPE(restype, *[CTYPE[a] for a in args])(img.slide + table[name])
        vals = values.split(",") if args else []
        conv = []
        for a, v in zip(args, vals):
            conv.append(int(v) if a == "i" else float(v) if a == "d" else v.encode())
        sys.stdout.flush()
        r = fn(*conv)
        if ret == "s":
            print(r.decode())
        elif ret == "d":
            print(f"{r:.3f}")
        elif ret == "i":
            print(r)
        sys.stdout.flush()
    machorun.LIBC.exit(0)


main()
