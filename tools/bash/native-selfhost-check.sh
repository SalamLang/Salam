#!/bin/sh
# Builds the compiler with the native backend and checks that the result
# behaves exactly like the compiler that built it.
#
# The reference compiler emits the IR for compiler/main.salam; the
# native-built compiler must then emit the same IR, byte for byte, apart
# from the embedded build date. That exercises the lexer, parser, type
# checker and llvmgen all compiled by nativegen, which is a far stronger
# check than any single test program.
#
# Usage: tools/bash/native-selfhost-check.sh <salam>

set -eu
SALAM=$1
case "$SALAM" in /*) ;; *) SALAM="$(pwd)/$SALAM" ;; esac
root=$(pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
cd "$work"
"$SALAM" build "$root/compiler/main.salam" --backend=native --release \
    --output="$work/salam-native" --log-level=error
"$SALAM" llvm "$root/compiler/main.salam" -o "$work/ref.ll" --log-level=error
start=$(date +%s)
"$work/salam-native" llvm "$root/compiler/main.salam" -o "$work/out.ll" --log-level=error
end=$(date +%s)
grep -v ' UTC\\00"$' "$work/ref.ll" >"$work/ref.trim"
grep -v ' UTC\\00"$' "$work/out.ll" >"$work/out.trim"
if ! cmp -s "$work/ref.trim" "$work/out.trim"; then
    echo "native-built compiler emitted different IR:"
    diff "$work/ref.trim" "$work/out.trim" | head -20
    exit 1
fi
echo "native-built compiler reproduces the reference IR ($((end - start))s)"
