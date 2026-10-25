#!/bin/sh
# Cross-compiles test programs for Windows with the native backend and runs
# them under wine, comparing stdout with each program's .out file.
#
# This is the same comparison the llvm/ test group makes (stdout only), so
# it suits tests/en/llvm/. Needs x86_64-w64-mingw32-gcc and wine on PATH.
#
# Usage: tools/bash/native-windows-check.sh <salam> <file.salam>...

set -u
SALAM=$1
shift
case "$SALAM" in /*) ;; *) SALAM="$(pwd)/$SALAM" ;; esac
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
export WINEDEBUG=-all
pass=0
fail=0
for f in "$@"; do
    case "$f" in /*) ;; *) f="$(pwd)/$f" ;; esac
    b=$(basename "$f" .salam)
    exp="${f%.salam}.out"
    [ -f "$exp" ] || continue
    d="$work/$b"
    mkdir -p "$d"
    if ! (cd "$d" && "$SALAM" build "$f" --backend=native --target=x86_64-w64-windows-gnu \
        --output="$d/t.exe" --log-level=error >"$d/build.log" 2>&1); then
        echo "FAIL $b (build)"
        head -5 "$d/build.log"
        fail=$((fail + 1))
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
echo "windows check: $pass passed, $fail failed"
[ "$fail" -eq 0 ]
