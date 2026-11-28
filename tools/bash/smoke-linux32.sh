#!/bin/sh
set -eu

[ $# -eq 3 ] || {
    echo "usage: $0 <cross|native> <i686|armhf> <salam>" >&2
    exit 2
}
MODE=$1
ARCH=$2
SALAM=$3
case "$SALAM" in /*) ;; *) SALAM="$(pwd)/$SALAM" ;; esac
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
export SALAM_STD="$ROOT/std"

case $ARCH in
i686) OWN=i686-linux-musl ;;
armhf) OWN=arm-linux-musleabihf ;;
*)
    echo "error: unknown architecture '$ARCH'" >&2
    exit 2
    ;;
esac

PROGRAMS="$ROOT/tests/en/llvm/ll_fib.salam $ROOT/tests/en/llvm/ll_hashmap.salam $ROOT/tests/en/llvm/ll_float.salam"

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT INT TERM
export SALAM_CACHE="$work/cache"
printf 'func main:\n    println "ok-llvm"\nend\n' >"$work/smoke.salam"
cd "$work"

expect_ok() {
    got=$("$@")
    [ "$got" = ok-llvm ] || {
        echo "error: '$*' printed '$got' instead of ok-llvm" >&2
        exit 1
    }
}

"$SALAM" version

# shellcheck disable=SC2086
case $MODE in
cross)
    sh "$ROOT/tools/bash/llvm-hardfloat-check.sh" "$SALAM"
    for target in x86_64-linux-musl aarch64-linux-musl i686-linux-musl arm-linux-musleabihf \
        riscv64-linux-musl x86_64-w64-windows-gnu wasm32-wasi; do
        echo "emit-obj $target"
        "$SALAM" llvm smoke.salam --target="$target" --emit-obj -o "$target.o" --log-level=warn
        [ -s "$target.o" ]
    done
    for target in "$OWN" x86_64-linux-musl; do
        echo "build and run $target with an empty PATH"
        env PATH=/nonexistent "$SALAM" build smoke.salam --backend=llvm --linker=system \
            --target="$target" --output="smoke-$target" --log-level=warn
        expect_ok "./smoke-$target"
    done
    echo "build x86_64-w64-windows-gnu with an empty PATH"
    env PATH=/nonexistent "$SALAM" build smoke.salam --backend=llvm --linker=system \
        --target=x86_64-w64-windows-gnu --output=smoke.exe --log-level=warn
    [ -s smoke.exe ]
    NATIVE_HOST_FLAGS="--backend=llvm --linker=system --target=$OWN" \
        sh "$ROOT/tools/bash/native-host-check.sh" "$SALAM" $PROGRAMS
    SALAM_LLVM_JOBS=4 SALAM_LLVM_PARTITION_MIN=1 \
        NATIVE_HOST_FLAGS="--backend=llvm --linker=system --target=$OWN" \
        sh "$ROOT/tools/bash/native-host-check.sh" "$SALAM" $PROGRAMS
    echo "run through the JIT"
    expect_ok "$SALAM" llvm smoke.salam --jit --log-level=warn
    echo "jit 64-bit division, which calls the compiler runtime"
    cat >wide.salam <<'WIDE'
import os

func main:
    n := os.Env("SALAM_SMOKE_N").ToInt() as i64
    d := n + 6 as i64
    u := n as u64 + 3 as u64
    println (9000000000000 as i64) / d, (9000000000001 as i64) % d
    println (18000000000000000000 as u64) / u, (18000000000000000001 as u64) % u
end
WIDE
    printf '1285714285714 3\n4500000000000000000 1\n' >wide.out
    SALAM_SMOKE_N=1 "$SALAM" llvm wide.salam --jit --log-level=warn >jit.out
    diff wide.out jit.out
    for f in $PROGRAMS; do
        echo "jit ${f##*/}"
        "$SALAM" llvm "$f" --jit --log-level=warn >jit.out
        diff "${f%.salam}.out" jit.out
    done
    ;;
native)
    "$SALAM" llvm smoke.salam --exec -o smoke-exec --log-level=warn
    expect_ok ./smoke-exec
    "$SALAM" build smoke.salam --output=smoke-default --log-level=warn
    expect_ok ./smoke-default
    NATIVE_HOST_FLAGS="--backend=llvm --linker=system" \
        sh "$ROOT/tools/bash/native-host-check.sh" "$SALAM" $PROGRAMS
    ;;
*)
    echo "error: unknown mode '$MODE'" >&2
    exit 2
    ;;
esac
echo "linux32 $MODE smoke test passed for $ARCH"
