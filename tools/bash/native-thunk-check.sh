#!/bin/sh
# Checks the built-in linker's branch thunks: an ARM B to a Thumb function, a
# Thumb B.W to an ARM function, and ARM, Thumb and AArch64 branches past their
# reach. Each case is a small assembly file, linked by compiler/nativelink and
# run under the emulator; it exits with a known status only if every branch
# landed; on ARM the thunk area must also get its own .ARM.exidx entry. An
# ARM64 PE branch past 128 MB is linked too (it cannot run here),
# and so is an arm64 Mach-O image with a far call and a far stub; for that
# one each bl is followed through its thunk and must reach its target.
#
# Needs clang for the assembly and python3 for the Mach-O case. The far cases
# write images of 20 to 130 MB.
#
# Usage: tools/bash/native-thunk-check.sh <salam> <arm|aarch64> <runner>
#   e.g. tools/bash/native-thunk-check.sh ./salam arm qemu-arm-static

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
ARCH=$2
RUNNER=$3
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
cd "$work" || exit 1
ln -s "$ROOT/compiler" compiler
cat >link.salam <<'EOF'
import os
import str

include nl "compiler/nativelink/nativelink.salam"

func main:
    a := os.Args()
    mut l := nl.New()
    if str.Equals(a.get(1), "pe"):
        nl.SetWindows(l)
    end
    nl.AddInput(l, a.get(3))
    if not nl.Link(l, a.get(2)):
        printerrln nl.Error(l)
        ret 1
    end
    ret 0
end
EOF
if ! "$SALAM" build link.salam --output=link --log-level=error >build.log 2>&1; then
    cat build.log
    echo "native thunk check: the link driver did not build"
    exit 1
fi

cat >a2t.s <<'EOF'
.syntax unified
.arm
.text
.global _start
_start:
    bl helper
    b thumbfn
.global helper
helper:
    bx lr
.thumb
.thumb_func
.global thumbfn
thumbfn:
    movs r0, #42
    movs r7, #1
    svc #0
EOF
cat >t2a.s <<'EOF'
.syntax unified
.thumb
.text
.thumb_func
.global _start
_start:
    b.w armfn
.arm
.global armfn
armfn:
    mov r0, #43
    mov r7, #1
    svc #0
EOF
cat >armfar.s <<'EOF'
.syntax unified
.arm
.text
.global _start
_start:
    bl far
    mov r7, #1
    svc #0
.section .text.far,"ax"
    .space 40*1024*1024
.Lfar:
    mov r0, #44
    bx lr
far:
    b .Lfar
EOF
cat >exfar.s <<'EOF'
.syntax unified
.arm
.text
.global _start
.fnstart
_start:
    bl far
    mov r7, #1
    svc #0
.fnend
.global __aeabi_unwind_cpp_pr0
__aeabi_unwind_cpp_pr0:
    bx lr
.section .text.far,"ax"
    .space 40*1024*1024
.global far
.fnstart
far:
    mov r0, #48
    bx lr
.fnend
EOF
cat >exidx.py <<'EOF'
import struct
import sys

b = open(sys.argv[1], "rb").read()
shoff, = struct.unpack_from("<I", b, 32)
shentsize, shnum = struct.unpack_from("<HH", b, 46)
ents = []
for k in range(shnum):
    sh = shoff + k * shentsize
    stype, = struct.unpack_from("<I", b, sh + 4)
    if stype == 0x70000001:
        addr, off, size = struct.unpack_from("<III", b, sh + 12)
        for e in range(0, size, 8):
            w0, w1 = struct.unpack_from("<II", b, off + e)
            fn = (addr + e + (w0 - (w0 >> 30 << 31))) & 0xFFFFFFFF
            ents.append((fn, w1))
ok = len(ents) == 3 and ents[0][0] < ents[1][0] < ents[2][0] and ents[1][1] == 0x80B0B0B0
sys.exit(0 if ok else 1)
EOF
cat >thfar.s <<'EOF'
.syntax unified
.thumb
.text
.thumb_func
.global _start
_start:
    bl far
    movs r7, #1
    svc #0
.section .text.far,"ax"
    .space 20*1024*1024
.thumb_func
.global far
far:
    movs r0, #46
    bx lr
EOF
cat >a64far.s <<'EOF'
.text
.global _start
_start:
    bl .Lfar
    mov x8, #93
    svc #0
.section .text.far,"ax"
    .space 130*1024*1024
.Lfar:
    mov x0, #45
    ret
EOF
cat >pefar.s <<'EOF'
.text
.globl mainCRTStartup
mainCRTStartup:
    bl far
    ret
.section .text$far,"xr"
    .space 130*1024*1024
far:
    ret
EOF
cat >macfar.s <<'EOF'
.text
.globl _main
_main:
    stp x29, x30, [sp, #-16]!
    bl _far
    bl _puts
    ldp x29, x30, [sp], #16
    ret
.section __TEXT,__far,regular,pure_instructions
    .space 130*1024*1024
.globl _far
_far:
    mov x0, #47
    ret
EOF
cat >macfar.py <<'EOF'
import struct
import sys

b = open(sys.argv[1], "rb").read()
base = 0x100000000
ncmds = struct.unpack_from("<I", b, 16)[0]
at = 32
syms = {}
stubs = None
for _ in range(ncmds):
    cmd, size = struct.unpack_from("<II", b, at)
    if cmd == 0x19:
        nsect = struct.unpack_from("<I", b, at + 64)[0]
        for k in range(nsect):
            sh = at + 72 + 80 * k
            if b[sh : sh + 7] == b"__stubs":
                stubs = struct.unpack_from("<Q", b, sh + 32)[0]
    if cmd == 0x2:
        symoff, nsyms, stroff = struct.unpack_from("<III", b, at + 8)
        for k in range(nsyms):
            strx, _t, _s, _d, val = struct.unpack_from("<IBBHQ", b, symoff + 16 * k)
            name = b[stroff + strx : b.index(b"\0", stroff + strx)].decode()
            syms[name] = val
    at += size


def word(a):
    return struct.unpack_from("<I", b, a - base)[0]


def sx(v, n):
    return v - (1 << n) if (v >> (n - 1)) & 1 else v


def follow(a):
    i = word(a)
    assert i >> 26 == 0x25, hex(i)
    t = a + sx(i & 0x3FFFFFF, 26) * 4
    i1, i2, i3 = word(t), word(t + 4), word(t + 8)
    assert i1 & 0x9F00001F == 0x90000010 and i2 & 0xFFC003FF == 0x91000210 and i3 == 0xD61F0200
    pg = ((i1 >> 29) & 3) | (((i1 >> 5) & 0x7FFFF) << 2)
    return (t & ~0xFFF) + sx(pg, 21) * 4096 + ((i2 >> 10) & 0xFFF)


m = syms["_main"]
sys.exit(0 if follow(m + 4) == syms["_far"] and follow(m + 8) == stubs else 1)
EOF

ok=0
bad=0
run_case() {
    name=$1
    target=$2
    want=$3
    if ! clang --target="$target" -c "$name.s" -o "$name.o" 2>"$name.err"; then
        echo "FAIL $name: does not assemble: $(head -c 200 "$name.err")"
        bad=$((bad + 1))
        return
    fi
    if ! ./link elf "$name.out" "$name.o" >"$name.err" 2>&1; then
        echo "FAIL $name: $(head -c 200 "$name.err")"
        bad=$((bad + 1))
        return
    fi
    "$RUNNER" "./$name.out"
    got=$?
    if [ "$got" = "$want" ]; then
        ok=$((ok + 1))
    else
        echo "FAIL $name: exit status $got, expected $want"
        bad=$((bad + 1))
    fi
    rm -f "$name.out" "$name.o"
}

case "$ARCH" in
arm)
    run_case a2t armv7a-linux-gnueabihf 42
    run_case t2a armv7a-linux-gnueabihf 43
    run_case armfar armv7a-linux-gnueabihf 44
    run_case thfar armv7a-linux-gnueabihf 46
    got=none
    if clang --target=armv7a-linux-gnueabihf -c exfar.s -o exfar.o && ./link elf exfar.out exfar.o; then
        "$RUNNER" ./exfar.out
        got=$?
    fi
    if [ "$got" = 48 ] && python3 exidx.py exfar.out; then
        ok=$((ok + 1))
    else
        echo "FAIL exfar: the thunk area has no .ARM.exidx entry of its own"
        bad=$((bad + 1))
    fi
    ;;
aarch64)
    run_case a64far aarch64-linux-gnu 45
    if clang --target=aarch64-w64-windows-gnu -c pefar.s -o pefar.obj && ./link pe pefar.exe pefar.obj; then
        ok=$((ok + 1))
    else
        echo "FAIL pefar"
        bad=$((bad + 1))
    fi
    if clang --target=arm64-apple-macos11 -c macfar.s -o macfar.o && ./link elf macfar.out macfar.o && python3 macfar.py macfar.out; then
        ok=$((ok + 1))
    else
        echo "FAIL macfar"
        bad=$((bad + 1))
    fi
    ;;
*)
    echo "native thunk check: no cases for $ARCH"
    exit 2
    ;;
esac
echo "native thunk check ($ARCH): $ok ok, $bad failed"
[ "$bad" = 0 ]
