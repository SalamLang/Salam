#!/bin/sh
# Rebuild compiler/salam from source (self-hosted: compiler/main.salam,
# compiled by the CURRENT compiler/salam binary). There is no C source
# anymore, so this is not a "compile from scratch" bootstrap — it always
# needs a working compiler/salam to start from (the one already tracked in
# git). Run this after editing any compiler/*.salam file to refresh the
# tracked binary.
#
# Builds to a temp file first and only replaces compiler/salam on success,
# so a broken in-progress change can't leave you without a working compiler.

set -e
. "$(dirname "$0")/lib.sh" # anchors cwd at the repo root

SALAM_BIN="${SALAM:-./compiler/salam}"
if [ ! -x "$SALAM_BIN" ]; then
    echo "error: $SALAM_BIN not found or not executable — need a working" >&2
    echo "       bootstrap compiler to self-host a new one from." >&2
    exit 1
fi

TMP_OUT="compiler/salam.new.$$"
trap 'rm -f "$TMP_OUT"' EXIT

echo ">> self-hosting: $SALAM_BIN build compiler/main.salam --output=$TMP_OUT"
SALAM_STD="${SALAM_STD:-$(pwd)}" "$SALAM_BIN" build compiler/main.salam --output="$TMP_OUT"

if [ ! -x "$TMP_OUT" ]; then
    echo "error: self-hosted build did not produce an executable" >&2
    exit 1
fi

if ! "$TMP_OUT" version >/dev/null 2>&1; then
    echo "error: freshly built $TMP_OUT failed 'salam version' — not replacing compiler/salam" >&2
    exit 1
fi

mv "$TMP_OUT" compiler/salam
chmod +x compiler/salam
echo "built compiler/salam (self-hosted)"
