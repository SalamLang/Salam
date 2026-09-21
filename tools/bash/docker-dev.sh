#!/bin/sh

set -eu

JOBS="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)"
SEED=${SALAM_SEED:-salam}
OUT=${OUT:-./salam}

build() {
    sh tools/bash/build-selfhost.sh --seed "$SEED" --output "$OUT"
}

command -v "$SEED" >/dev/null 2>&1 || [ -x "$SEED" ] || {
    echo "[salam] no seed compiler '$SEED' (run install.sh, or set SALAM_SEED)" >&2
    exit 1
}

echo "[salam] initial build ($OUT, -j$JOBS) ..."
build || echo "[salam] initial build failed - waiting for changes"

ENTR_FLAGS=-dc
if [ ! -t 0 ]; then
    ENTR_FLAGS=-dcn
fi

echo "[salam] watching compiler/ and std/ for changes (Ctrl-C to stop) ..."
while true; do
    find compiler std -type f -name '*.salam' |
        entr "$ENTR_FLAGS" sh -c 'sh tools/bash/build-selfhost.sh --seed "'"$SEED"'" --output "'"$OUT"'" && echo "[salam] rebuilt '"$OUT"'"' ||
        true
done
