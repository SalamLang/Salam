#!/usr/bin/env python3
"""Run a linked x86_64 Mach-O executable on x86_64 Linux by loading it into
this process and binding its libSystem imports to glibc."""

import ctypes
import mmap
import struct
import sys

LIBC = ctypes.CDLL(None)

SPECIAL_VAR = {
    "___stdoutp": "stdout",
    "___stderrp": "stderr",
    "___stdinp": "stdin",
    "_environ": "environ",
}
SPECIAL_FN = {
    "___error": "__errno_location",
    "___exit": "_exit",
    "dyld_stub_binder": "abort",
    "_atexit": "on_exit",
}


def uleb(b, p):
    r = 0
    s = 0
    while True:
        x = b[p]
        p += 1
        r |= (x & 0x7F) << s
        s += 7
        if x < 0x80:
            return r, p


def sleb(b, p):
    r = 0
    s = 0
    while True:
        x = b[p]
        p += 1
        r |= (x & 0x7F) << s
        s += 7
        if x < 0x80:
            if x & 0x40:
                r -= 1 << s
            return r, p


def addr_of_func(name):
    try:
        return ctypes.cast(getattr(LIBC, name), ctypes.c_void_p).value
    except AttributeError:
        return None


def addr_of_var(name):
    try:
        return ctypes.addressof(ctypes.c_void_p.in_dll(LIBC, name))
    except ValueError:
        return None


def resolve(name):
    if name in SPECIAL_VAR:
        a = addr_of_var(SPECIAL_VAR[name])
        if a:
            return a
    if name in SPECIAL_FN:
        a = addr_of_func(SPECIAL_FN[name])
        if a:
            return a
    n = name.removeprefix("_")
    for cand in (n, n.replace("$INODE64", "").replace("$UNIX2003", "")):
        a = addr_of_func(cand) or addr_of_var(cand)
        if a:
            return a
    sys.stderr.write("machorun: unresolved import " + name + "\n")
    return 0


class Image:
    """The loaded segments, addressed by their Mach-O virtual addresses."""

    def __init__(self, data, segs):
        self.segs = segs
        load = [s for s in segs if s[0] != "__PAGEZERO"]
        self.lo = min(s[1] for s in load)
        hi = max(s[1] + s[2] for s in load)
        size = (hi - self.lo + 0xFFF) & ~0xFFF
        prot = mmap.PROT_READ | mmap.PROT_WRITE | mmap.PROT_EXEC
        self.mem = mmap.mmap(-1, size, prot=prot)
        self.base = ctypes.addressof(ctypes.c_char.from_buffer(self.mem))
        self.slide = self.base - self.lo
        for s in load:
            dst = s[1] - self.lo
            dst_end = dst + s[4]
            src = s[3]
            src_end = src + s[4]
            self.mem[dst:dst_end] = data[src:src_end]

    def seg_addr(self, i):
        return self.segs[i][1]

    def put64(self, vaddr, v):
        o = vaddr - self.lo
        o_end = o + 8
        self.mem[o:o_end] = struct.pack("<Q", v & 0xFFFFFFFFFFFFFFFF)

    def get64(self, vaddr):
        o = vaddr - self.lo
        o_end = o + 8
        return struct.unpack("<Q", self.mem[o:o_end])[0]

    def rebase_at(self, a):
        self.put64(a, self.get64(a) + self.slide)


def parse(data):
    magic, cpu = struct.unpack_from("<Ii", data, 0)
    ncmds = struct.unpack_from("<I", data, 16)[0]
    if magic != 0xFEEDFACF or cpu != 0x01000007:
        raise SystemExit("machorun: not an x86_64 Mach-O")
    off = 32
    segs = []
    dyld = None
    entry = None
    for _ in range(ncmds):
        cmd, csize = struct.unpack_from("<II", data, off)
        if cmd == 0x19:
            n0 = off + 8
            n1 = off + 24
            name = bytes(data[n0:n1]).rstrip(b"\0").decode()
            segs.append((name,) + struct.unpack_from("<QQQQ", data, off + 24))
        elif cmd in (0x22, 0x80000022):
            dyld = struct.unpack_from("<10I", data, off + 8)
        elif cmd == 0x80000028:
            entry = struct.unpack_from("<Q", data, off + 8)[0]
        off += csize
    return segs, dyld, entry


def rebase(img, b, p, end):
    a = 0
    while p < end:
        op = b[p] & 0xF0
        imm = b[p] & 0x0F
        p += 1
        if op == 0x00:
            break
        if op == 0x20:
            o, p = uleb(b, p)
            a = img.seg_addr(imm) + o
        elif op == 0x30:
            o, p = uleb(b, p)
            a += o
        elif op == 0x40:
            a += imm * 8
        elif op in (0x50, 0x60):
            n = imm
            if op == 0x60:
                n, p = uleb(b, p)
            for _ in range(n):
                img.rebase_at(a)
                a += 8
        elif op == 0x70:
            img.rebase_at(a)
            o, p = uleb(b, p)
            a += 8 + o
        elif op == 0x80:
            n, p = uleb(b, p)
            sk, p = uleb(b, p)
            for _ in range(n):
                img.rebase_at(a)
                a += 8 + sk
        elif op != 0x10:
            raise SystemExit(f"machorun: bad rebase opcode {op:x}")


def bind_meta(b, p, op, sym, addend):
    if op == 0x20:
        _, p = uleb(b, p)
    elif op == 0x40:
        q = b.index(b"\0", p)
        sym = b[p:q].decode()
        p = q + 1
    elif op == 0x60:
        addend, p = sleb(b, p)
    return sym, addend, p


def bind(img, b, p, end, lazy):
    sym = ""
    addend = 0
    a = 0
    while p < end:
        op = b[p] & 0xF0
        imm = b[p] & 0x0F
        p += 1
        if op == 0x00:
            if lazy:
                continue
            break
        if op in (0x20, 0x40, 0x60):
            sym, addend, p = bind_meta(b, p, op, sym, addend)
        elif op in (0x70, 0x80):
            o, p = uleb(b, p)
            a = (img.seg_addr(imm) if op == 0x70 else a) + o
        elif op in (0x90, 0xA0, 0xB0):
            img.put64(a, resolve(sym) + addend)
            a += 8
            if op == 0xA0:
                o, p = uleb(b, p)
                a += o
            elif op == 0xB0:
                a += imm * 8
        elif op == 0xC0:
            n, p = uleb(b, p)
            sk, p = uleb(b, p)
            for _ in range(n):
                img.put64(a, resolve(sym) + addend)
                a += 8 + sk
        elif op not in (0x10, 0x30, 0x50):
            raise SystemExit(f"machorun: bad bind opcode {op:x}")


def link(img, data, dyld):
    rb_off, rb_size, bd_off, bd_size = dyld[0:4]
    lz_off, lz_size = dyld[6:8]
    rebase(img, data, rb_off, rb_off + rb_size)
    bind(img, data, bd_off, bd_off + bd_size, False)
    bind(img, data, lz_off, lz_off + lz_size, True)


def main():
    path = sys.argv[1]
    with open(path, "rb") as f:
        data = f.read()
    segs, dyld, entry = parse(data)
    img = Image(data, segs)
    if dyld:
        link(img, data, dyld)
    text = next(s for s in segs if s[0] == "__TEXT")
    proto = ctypes.CFUNCTYPE(
        ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_char_p)
    )
    fn = proto(img.base + (text[1] - img.lo) + entry)
    args = [path.encode()] + [a.encode() for a in sys.argv[2:]]
    argv = (ctypes.c_char_p * (len(args) + 1))(*args, None)
    sys.stdout.flush()
    rc = fn(len(args), argv)
    LIBC.exit(rc)


main()
