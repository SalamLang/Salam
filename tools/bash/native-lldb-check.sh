#!/bin/sh
# Checks debug info and unwind tables in macOS programs built with
# --backend=native -g and linked by the built-in linker. The programs are
# built on any host (`build`), then debugged under lldb on a Mac (`run`):
# lldb stops on a function and on source lines, prints arguments and locals
# (a struct split into pieces, a float argument, register locals, constants
# left after folding a loop), and walks the stack from the panic handler back
# through an inlined std function to main. The x86-64 program is checked
# without running it, since lldb hangs on it under Rosetta: its breakpoints
# must resolve to the right lines, the loop's folded locals must have
# constant locations there, and the image must carry __eh_frame.
#
# When clang and llvm-ar are there at build time it also links mix_arm64, a
# program that calls into a static archive of two C objects, one with DWARF 4
# and one with DWARF 5: lldb must stop in both C functions, read their locals
# and walk back into the Salam caller, so the DWARF of every object is kept;
# the archive also has a __thread variable, so the run checks Mach-O TLV too.
#
# Usage: tools/bash/native-lldb-check.sh build <salam> <dir>
#        tools/bash/native-lldb-check.sh run <dir>

set -u
MODE=$1
if [ "$MODE" = build ]; then
    SALAM=$2
    case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
    mkdir -p "$3"
    DIR=$(cd "$3" && pwd)
    cat >"$DIR/dg.salam" <<'SRC'
struct Pt:
    pub x: int = 0
    pub y: int = 0
end

noinline func area(p: Pt, scale: f64): f64:
    w := p.x * 2
    h := p.y + 1
    r := ((w * h) as f64) * scale
    ret r
end

noinline func walk(n: int, name: str): int:
    mut s := 0
    mut k := 3
    repeat n in i:
        s += i * k
        k += 1
    end
    println name
    ret s
end

noinline func boom(v: Vector<int>, at: int): int:
    ret v.get(at)
end

func main:
    p := Pt { x = 3, y = 4 }
    println area(p, 1.5)
    println walk(5, "hello")
    mut v := Vector {} as Vector<int>
    v.push(1)
    println boom(v, 5)
end
SRC
    for arch in arm64 x86_64; do
        if ! (cd "$DIR" && SALAM_LINKER=builtin "$SALAM" build dg.salam --backend=native -g \
            --target="$arch-apple-macos" --output="dg_$arch" --log-level=error >"build_$arch.log" 2>&1); then
            echo "native lldb check: building for $arch failed"
            sed -n 1,5p "$DIR/build_$arch.log"
            exit 1
        fi
    done
    echo "native lldb check: built $DIR/dg_arm64 and $DIR/dg_x86_64"
    AR=$(command -v llvm-ar || find /usr/bin -name 'llvm-ar-[0-9]*' 2>/dev/null | sort -V | tail -1)
    if ! command -v clang >/dev/null 2>&1 || [ -z "$AR" ]; then
        echo "native lldb check: no clang or llvm-ar, mix_arm64 not built"
        exit 0
    fi
    mkdir -p "$DIR/mix"
    cat >"$DIR/mix/mixc.c" <<'SRC'
int cadd(int a, int b) {
    int s = a + b;
    return s * 2;
}
__thread int tcount = 40;
int tls_bump(void) {
    tcount += 2;
    return tcount;
}
SRC
    cat >"$DIR/mix/mixd.c" <<'SRC'
int cmul(int a, int b) {
    int p = a * b;
    return p + 1;
}
SRC
    cat >"$DIR/mix/mix.salam" <<'SRC'
link static "libmix.a"

extern:
    func cadd(a: int, b: int): int
    func cmul(a: int, b: int): int
    func tls_bump(): int
end

noinline func walk(n: int): int:
    ret cmul(n, 7) + cadd(n, 1)
end

func main:
    println walk(5)
    println "tls", tls_bump()
end
SRC
    if ! (cd "$DIR/mix" && clang --target=arm64-apple-macos14 -gdwarf-4 -O0 -c mixc.c -o mixc.o &&
        clang --target=arm64-apple-macos14 -gdwarf-5 -O0 -c mixd.c -o mixd.o &&
        "$AR" rcs libmix.a mixc.o mixd.o &&
        SALAM_LINKER=builtin "$SALAM" build mix.salam --backend=native -g --target=arm64-apple-macos \
            --output="$DIR/mix_arm64" --log-level=error >build.log 2>&1); then
        echo "native lldb check: building mix_arm64 failed"
        sed -n 1,5p "$DIR/mix/build.log"
        exit 1
    fi
    echo "native lldb check: built $DIR/mix_arm64"
    exit 0
fi
if [ "$MODE" != run ]; then
    echo "usage: $0 build <salam> <dir> | run <dir>"
    exit 2
fi
DIR=$(cd "$2" && pwd)
fail=0
check() {
    if ! grep -qF -- "$2" "$1"; then
        echo "FAIL $3: expected '$2'"
        fail=$((fail + 1))
    fi
}
lim() {
    perl -e 'alarm shift; exec @ARGV' "$@"
}
out="$DIR/lldb_arm64.txt"
chmod +x "$DIR/dg_arm64"
(cd "$DIR" && lim 300 lldb --batch --no-lldbinit \
    -o "breakpoint set -n salam_area_Pt_f64" -o "breakpoint set -f dg.salam -l 9" \
    -o "breakpoint set -f dg.salam -l 21" -o "breakpoint set -n salam_panic" \
    -o run -o "frame variable" -o continue -o "frame variable" -o continue -o "frame variable" \
    -o continue -o "thread backtrace" -o kill -- "$DIR/dg_arm64" >"$out" 2>&1)
check "$out" "x = 3" "arm64 struct pieces"
check "$out" "y = 4" "arm64 struct pieces"
check "$out" "scale = 1.5" "arm64 float argument"
check "$out" "w = 6" "arm64 register local"
check "$out" "h = 5" "arm64 register local"
check "$out" "s = 60" "arm64 folded loop"
check "$out" "k = 8" "arm64 folded loop"
check "$out" "\`boom(v=" "arm64 backtrace through the inlined frame"
check "$out" "\`main at dg.salam" "arm64 backtrace reaches main"
if [ "$fail" -gt 0 ]; then
    cat "$out"
fi
f0=$fail
xo="$DIR/lldb_x86_64.txt"
(cd "$DIR" && lim 120 lldb --batch --no-lldbinit \
    -o "breakpoint set -n salam_area_Pt_f64" -o "breakpoint set -f dg.salam -l 21" \
    -o "image dump sections dg_x86_64" -- "$DIR/dg_x86_64" >"$xo" 2>&1)
check "$xo" "\`area + " "x86_64 function breakpoint"
check "$xo" "__TEXT.__eh_frame" "x86_64 unwind table"
at=$(sed -n 's/.*`walk + [0-9]* at dg.salam:21, address = \(0x[0-9a-f]*\).*/\1/p' "$xo")
if [ -z "$at" ]; then
    echo "FAIL x86_64 line breakpoint: dg.salam:21 does not resolve into walk"
    fail=$((fail + 1))
else
    (cd "$DIR" && lim 120 lldb --batch --no-lldbinit -o "image lookup -v -a $at" -- "$DIR/dg_x86_64" >>"$xo" 2>&1)
    check "$xo" 'name = "s", type = "i32"' "x86_64 folded loop"
    if ! grep -q 'name = "s".*DW_OP_consts +60' "$xo" || ! grep -q 'name = "k".*DW_OP_consts +8' "$xo"; then
        echo "FAIL x86_64 folded loop: s and k are not 60 and 8 at dg.salam:21"
        fail=$((fail + 1))
    fi
fi
if [ "$fail" -gt "$f0" ]; then
    cat "$xo"
fi
if [ -f "$DIR/mix_arm64" ]; then
    f1=$fail
    mo="$DIR/lldb_mix.txt"
    chmod +x "$DIR/mix_arm64"
    (cd "$DIR" && lim 300 lldb --batch --no-lldbinit \
        -o "breakpoint set -f mixd.c -l 3" -o "breakpoint set -f mixc.c -l 3" \
        -o run -o "frame variable" -o "thread backtrace" -o continue -o "frame variable" -o continue \
        -- "$DIR/mix_arm64" >"$mo" 2>&1)
    check "$mo" "p = 35" "mix DWARF 5 C local"
    check "$mo" "s = 6" "mix DWARF 4 C local"
    check "$mo" "\`walk(" "mix backtrace into the Salam caller"
    check "$mo" "tls 42" "mix thread-local from the C archive"
    if [ "$fail" -gt "$f1" ]; then
        cat "$mo"
    fi
else
    echo "native lldb check: no mix_arm64, the C archive case was skipped"
fi
echo "native lldb check: $fail failed"
[ "$fail" -eq 0 ]
