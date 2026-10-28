#!/bin/sh
# Cross-compiles test programs for Windows with the native backend and runs
# them under wine, comparing stdout with each program's .out file.
#
# This is the same comparison the llvm/ test group makes (stdout only), so
# it suits tests/en/llvm/. Needs wine on PATH, and the mingw-w64 runtime for
# the target (found by the built-in linker, or a cross compiler on PATH).
#
# NATIVE_WIN_TRIPLE picks the target (x86_64-w64-windows-gnu by default).
# With NATIVE_WIN_STAGE=<dir> the programs are only built, and each lands in
# <dir> as <name>.exe next to its <name>.out, for tools/bash/native-stage-run.sh
# to run on a Windows machine; this is how aarch64-w64-windows-gnu is tested.
#
# Usage: tools/bash/native-windows-check.sh <salam> <file.salam>...

set -u
SALAM=$1
shift
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
TRIPLE=${NATIVE_WIN_TRIPLE:-x86_64-w64-windows-gnu}
STAGE=${NATIVE_WIN_STAGE:-}
if [ -n "$STAGE" ]; then
    mkdir -p "$STAGE"
    STAGE=$(cd "$STAGE" && pwd)
fi
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
export WINEDEBUG=-all
pass=0
fail=0
staged=0
for f in "$@"; do
    case "$f" in /*) ;; *) f="$(pwd)/$f" ;; esac
    b=$(basename "$f" .salam)
    exp="${f%.salam}.out"
    [ -f "$exp" ] || continue
    d="$work/$b"
    mkdir -p "$d"
    if ! (cd "$d" && "$SALAM" build "$f" --backend=native --target="$TRIPLE" \
        --output="$d/t.exe" --log-level=error >"$d/build.log" 2>&1); then
        echo "FAIL $b (build)"
        head -5 "$d/build.log"
        fail=$((fail + 1))
        continue
    fi
    if [ -n "$STAGE" ]; then
        if cp "$d/t.exe" "$STAGE/$b.exe" && cp "$exp" "$STAGE/$b.out"; then
            staged=$((staged + 1))
        else
            echo "FAIL $b (staging)"
            fail=$((fail + 1))
        fi
        continue
    fi
    (cd "$d" && timeout 120 wine "$d/t.exe" </dev/null >"$d/run.out" 2>/dev/null)
    if [ "$(tr -d '\r' <"$d/run.out")" = "$(tr -d '\r' <"$exp")" ]; then
        pass=$((pass + 1))
    else
        echo "FAIL $b"
        diff "$exp" "$d/run.out" | head -6
        fail=$((fail + 1))
    fi
done
if [ -n "$STAGE" ]; then
    echo "windows check ($TRIPLE): $staged staged in $STAGE, $fail failed to build"
    [ "$fail" -eq 0 ]
    exit
fi
echo "windows check: $pass passed, $fail failed"
[ "$fail" -eq 0 ]
