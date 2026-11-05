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
# Needs clang for the assembly. The far cases write images of 20 to 130 MB, and
# tools/linker/thunk_check.salam reads the ARM unwind table and the Mach-O image.
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
if ! SALAM_STD="$ROOT/std" "$SALAM" build "$ROOT/tools/linker/thunk_check.salam" --output=thunk_check --log-level=error >build.log 2>&1; then
    cat build.log
    echo "native thunk check: the image checker did not build"
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
    if [ "$got" = 48 ] && ./thunk_check exidx exfar.out; then
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
    if clang --target=arm64-apple-macos11 -c macfar.s -o macfar.o && ./link elf macfar.out macfar.o && ./thunk_check macfar macfar.out; then
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
