#!/usr/bin/env python3
"""Run a linked x86_64 Mach-O executable on x86_64 Linux by loading it into
this process and binding its libSystem imports to glibc."""
import ctypes, mmap, struct, sys

def uleb(b, p):
    r = 0; s = 0
    while True:
        x = b[p]; p += 1
        r |= (x & 0x7f) << s; s += 7
        if x < 0x80:
            return r, p

def sleb(b, p):
    r = 0; s = 0
    while True:
        x = b[p]; p += 1
        r |= (x & 0x7f) << s; s += 7
        if x < 0x80:
            if x & 0x40:
                r -= 1 << s
            return r, p

libc = ctypes.CDLL(None)

def addr_of_func(name):
    try:
        return ctypes.cast(getattr(libc, name), ctypes.c_void_p).value
    except AttributeError:
        return None

def addr_of_var(name):
    try:
        return ctypes.addressof(ctypes.c_void_p.in_dll(libc, name))
    except ValueError:
        return None

SPECIAL_VAR = {'___stdoutp': 'stdout', '___stderrp': 'stderr', '___stdinp': 'stdin', '_environ': 'environ'}
SPECIAL_FN = {'___error': '__errno_location', '___exit': '_exit', 'dyld_stub_binder': 'abort', '_atexit': 'on_exit'}

def resolve(name):
    if name in SPECIAL_VAR:
        a = addr_of_var(SPECIAL_VAR[name])
        if a:
            return a
    if name in SPECIAL_FN:
        a = addr_of_func(SPECIAL_FN[name])
        if a:
            return a
    n = name[1:] if name.startswith('_') else name
    for cand in (n, n.replace('$INODE64', '').replace('$UNIX2003', '')):
        a = addr_of_func(cand)
        if a:
            return a
        a = addr_of_var(cand)
        if a:
            return a
    sys.stderr.write('machorun: unresolved import ' + name + '\n')
    return 0

def main():
    path = sys.argv[1]
    data = open(path, 'rb').read()
    magic, cpu, sub, ftype, ncmds, sizecmds, flags, _ = struct.unpack_from('<IiiIIIII', data, 0)
    assert magic == 0xfeedfacf and cpu == 0x01000007, 'not an x86_64 Mach-O'
    off = 32
    segs = []; dyld = None; entry = None
    for _ in range(ncmds):
        cmd, csize = struct.unpack_from('<II', data, off)
        if cmd == 0x19:
            name = data[off + 8:off + 24].rstrip(b'\0').decode()
            vmaddr, vmsize, fileoff, filesize = struct.unpack_from('<QQQQ', data, off + 24)
            segs.append((name, vmaddr, vmsize, fileoff, filesize))
        elif cmd in (0x22, 0x80000022):
            dyld = struct.unpack_from('<10I', data, off + 8)
        elif cmd == 0x80000028:
            entry = struct.unpack_from('<Q', data, off + 8)[0]
        off += csize
    load = [s for s in segs if s[0] != '__PAGEZERO']
    lo = min(s[1] for s in load)
    hi = max(s[1] + s[2] for s in load)
    size = (hi - lo + 0xfff) & ~0xfff
    mem = mmap.mmap(-1, size, prot=mmap.PROT_READ | mmap.PROT_WRITE | mmap.PROT_EXEC)
    base = ctypes.addressof(ctypes.c_char.from_buffer(mem))
    slide = base - lo
    for name, vmaddr, vmsize, fileoff, filesize in load:
        mem[vmaddr - lo:vmaddr - lo + filesize] = data[fileoff:fileoff + filesize]
    def seg_addr(i):
        return segs[i][1]
    def put64(vaddr, v):
        o = vaddr - lo
        mem[o:o + 8] = struct.pack('<Q', v & 0xffffffffffffffff)
    def get64(vaddr):
        o = vaddr - lo
        return struct.unpack('<Q', mem[o:o + 8])[0]
    if dyld:
        rb_off, rb_size, bd_off, bd_size, wk_off, wk_size, lz_off, lz_size, ex_off, ex_size = dyld
        b = data; p = rb_off; end = rb_off + rb_size; typ = 1; seg = 0; a = 0
        while p < end:
            op = b[p] & 0xf0; imm = b[p] & 0x0f; p += 1
            if op == 0x00:
                break
            elif op == 0x10:
                typ = imm
            elif op == 0x20:
                seg = imm; o, p = uleb(b, p); a = seg_addr(seg) + o
            elif op == 0x30:
                o, p = uleb(b, p); a += o
            elif op == 0x40:
                a += imm * 8
            elif op == 0x50:
                for _ in range(imm):
                    put64(a, get64(a) + slide); a += 8
            elif op == 0x60:
                n, p = uleb(b, p)
                for _ in range(n):
                    put64(a, get64(a) + slide); a += 8
            elif op == 0x70:
                put64(a, get64(a) + slide); o, p = uleb(b, p); a += 8 + o
            elif op == 0x80:
                n, p = uleb(b, p); sk, p = uleb(b, p)
                for _ in range(n):
                    put64(a, get64(a) + slide); a += 8 + sk
            else:
                raise SystemExit('machorun: bad rebase opcode %x' % op)
        for start, length in ((bd_off, bd_size), (lz_off, lz_size)):
            p = start; end = start + length; sym = ''; addend = 0; seg = 0; a = 0
            while p < end:
                op = b[p] & 0xf0; imm = b[p] & 0x0f; p += 1
                if op == 0x00:
                    if start == lz_off:
                        continue
                    break
                elif op in (0x10, 0x20, 0x30):
                    if op == 0x20:
                        _, p = uleb(b, p)
                elif op == 0x40:
                    q = b.index(b'\0', p); sym = b[p:q].decode(); p = q + 1
                elif op == 0x50:
                    pass
                elif op == 0x60:
                    addend, p = sleb(b, p)
                elif op == 0x70:
                    seg = imm; o, p = uleb(b, p); a = seg_addr(seg) + o
                elif op == 0x80:
                    o, p = uleb(b, p); a += o
                elif op == 0x90:
                    put64(a, resolve(sym) + addend); a += 8
                elif op == 0xA0:
                    put64(a, resolve(sym) + addend); o, p = uleb(b, p); a += 8 + o
                elif op == 0xB0:
                    put64(a, resolve(sym) + addend); a += 8 + imm * 8
                elif op == 0xC0:
                    n, p = uleb(b, p); sk, p = uleb(b, p)
                    for _ in range(n):
                        put64(a, resolve(sym) + addend); a += 8 + sk
                else:
                    raise SystemExit('machorun: bad bind opcode %x' % op)
    text = [s for s in segs if s[0] == '__TEXT'][0]
    fn = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_char_p))(base + (text[1] - lo) + entry)
    args = [path.encode()] + [a.encode() for a in sys.argv[2:]]
    argv = (ctypes.c_char_p * (len(args) + 1))(*args, None)
    sys.stdout.flush()
    rc = fn(len(args), argv)
    libc.exit(rc)

main()
