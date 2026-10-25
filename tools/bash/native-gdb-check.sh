#!/bin/sh
# Debugs a small program built with --backend=native -g under gdb and checks
# what gdb prints: locals and arguments in registers, a struct split into
# pieces, constants left after folding a loop, and a backtrace through an
# inlined std function. It runs once per linker.
#
# With a target triple and a runner it cross-builds instead (built-in linker
# only, against SALAM_MUSL_SYSROOT) and debugs through gdb-multiarch and the
# runner's gdb stub, e.g. qemu-aarch64-static -g. Use `env` as the runner for
# a target the host runs directly (i686).
#
# Usage: tools/bash/native-gdb-check.sh <salam> [<triple> <runner>]

set -u
SALAM=$1
case "$SALAM" in /*) ;; *) SALAM="$(pwd)/$SALAM" ;; esac
TRIPLE=${2:-}
RUNNER=${3:-}
GDB=gdb
if [ -n "$TRIPLE" ] && [ "$RUNNER" != env ]; then
    GDB=gdb-multiarch
fi
if ! command -v "$GDB" >/dev/null 2>&1; then
    echo "native gdb check: $GDB not found, skipped"
    exit 0
fi
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
cat >"$work/dg.salam" <<'SRC'
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
fail=0
check() {
    if ! grep -qF -- "$2" "$1"; then
        echo "FAIL $3: expected '$2'"
        fail=$((fail + 1))
    fi
}
linkers="builtin system"
tgt=
if [ -n "$TRIPLE" ]; then
    linkers=builtin
    tgt=--target=$TRIPLE
fi
for lk in $linkers; do
    # shellcheck disable=SC2086
    if ! (cd "$work" && SALAM_LINKER=$lk "$SALAM" build dg.salam --backend=native -g $tgt --output="dg_$lk" --log-level=error >"$work/build_$lk.log" 2>&1); then
        echo "FAIL $lk: build failed"
        sed -n 1,5p "$work/build_$lk.log"
        fail=$((fail + 1))
        continue
    fi
    out="$work/gdb_$lk.txt"
    if [ -z "$TRIPLE" ] || [ "$RUNNER" = env ]; then
        set -- -ex run
    else
        port=$((20000 + $$ % 20000))
        # shellcheck disable=SC2086
        (cd "$work" && exec $RUNNER -g "$port" "./dg_$lk" >/dev/null 2>&1) &
        sleep 1
        set -- -ex "target remote :$port" -ex continue
    fi
    (cd "$work" && timeout 120 "$GDB" -batch -nx \
        -ex "break salam_area_Pt_f64" -ex "break dg.salam:9" -ex "break dg.salam:21" -ex "break salam_panic" \
        "$@" -ex "info args" -ex continue -ex "info locals" -ex continue -ex "info locals" \
        -ex continue -ex bt -ex kill "./dg_$lk" >"$out" 2>&1)
    wait
    check "$out" "p = {x = 3, y = 4}" "$lk struct pieces"
    check "$out" "scale = 1.5" "$lk float argument"
    check "$out" "w = 6" "$lk register local"
    check "$out" "h = 5" "$lk register local"
    check "$out" "s = 60" "$lk folded loop"
    check "$out" "k = 8" "$lk folded loop"
    check "$out" "in get (" "$lk inlined frame"
    check "$out" "boom (" "$lk caller of inlined frame"
done
echo "native gdb check: $fail failed"
[ "$fail" -eq 0 ]
