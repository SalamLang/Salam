#!/bin/sh
# The compiler is the program --icf folds the most in, so this links it with
# --icf and has it reproduce the reference IR on the machine it was built for.
#
# build: links the compiler for <triple> into <dir>, and fails unless it came
#        out smaller than the one in <package.tar.gz>, which was not folded.
# run:   compiles compiler/main.salam with it, from the repository root, and
#        compares the IR with <ref.ll>, ignoring the build date.
#
# Usage: tools/bash/native-icf-compiler.sh build <salam> <triple> <package.tar.gz> <dir>
#        tools/bash/native-icf-compiler.sh run <dir> <ref.ll>

set -eu
MODE=$1
shift
if [ "$MODE" = build ]; then
    SALAM=$1
    TRIPLE=$2
    PKG=$3
    DIR=$4
    EXE=salam
    case "$TRIPLE" in *windows*) EXE=salam.exe ;; esac
    mkdir -p "$DIR"
    "$SALAM" build compiler/main.salam --backend=native --release --icf --target="$TRIPLE" \
        --output="$DIR/$EXE" --log-level=error
    top=$(basename "$PKG" .tar.gz)
    plain=$(tar -xzOf "$PKG" "$top/$EXE" | wc -c)
    folded=$(wc -c <"$DIR/$EXE")
    echo "compiler for $TRIPLE: $plain bytes, $folded with --icf"
    [ "$folded" -lt "$plain" ]
    exit
fi
DIR=$1
REF=$2
EXE=$DIR/salam
[ -f "$EXE" ] || EXE=$DIR/salam.exe
chmod +x "$EXE"
std="$(pwd)/std"
if command -v cygpath >/dev/null 2>&1; then std="$(cygpath -m "$std")"; fi
SALAM_STD="$std" "$EXE" llvm compiler/main.salam --target=x86_64-linux-gnu -o "$DIR/out.ll" --log-level=error
grep -v ' UTC\\00"$' "$REF" >"$DIR/ref.trim"
grep -v ' UTC\\00"$' "$DIR/out.ll" | tr -d '\r' >"$DIR/out.trim"
if ! cmp -s "$DIR/ref.trim" "$DIR/out.trim"; then
    diff "$DIR/ref.trim" "$DIR/out.trim" | head -20
    exit 1
fi
echo "the compiler linked with --icf reproduces the reference IR on this host"
