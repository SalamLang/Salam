#!/bin/sh
# Checks debug info and unwind tables in macOS programs built with
# --backend=native -g and linked by the built-in linker. The programs are
# built on any host (`build`), then debugged under lldb on a Mac (`run`):
# lldb stops on a function and on source lines, prints arguments and locals
# (a struct split into pieces, a float argument, register locals, constants
# left after folding a loop), and walks the stack from the panic handler back
# through an inlined std function to main.
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
arches=arm64
if arch -x86_64 /usr/bin/true 2>/dev/null; then
    arches="arm64 x86_64"
fi
for a in $arches; do
    exe="$DIR/dg_$a"
    chmod +x "$exe"
    out="$DIR/lldb_$a.txt"
    (cd "$DIR" && lldb --batch --no-lldbinit \
        -o "breakpoint set -n salam_area_Pt_f64" -o "breakpoint set -f dg.salam -l 9" \
        -o "breakpoint set -f dg.salam -l 21" -o "breakpoint set -n salam_panic" \
        -o run -o "frame variable" -o continue -o "frame variable" -o continue -o "frame variable" \
        -o continue -o "thread backtrace" -o kill -- "$exe" >"$out" 2>&1)
    check "$out" "x = 3" "$a struct pieces"
    check "$out" "y = 4" "$a struct pieces"
    check "$out" "scale = 1.5" "$a float argument"
    check "$out" "w = 6" "$a register local"
    check "$out" "h = 5" "$a register local"
    check "$out" "s = 60" "$a folded loop"
    check "$out" "k = 8" "$a folded loop"
    check "$out" "salam_boom" "$a backtrace through the inlined frame"
    check "$out" "main" "$a backtrace reaches main"
    if [ "$fail" -gt 0 ]; then
        cat "$out"
    fi
done
echo "native lldb check ($arches): $fail failed"
[ "$fail" -eq 0 ]
