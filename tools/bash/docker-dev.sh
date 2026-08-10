#!/bin/sh
# Live-rebuild loop for the Salam compiler inside the `dev` container.

set -eu

JOBS="$(nproc 2>/dev/null || echo 1)"

echo "[salam] initial build (make -C c OUTDIR=.. -j$JOBS) ..."
make -C c OUTDIR=.. -j"$JOBS" || echo "[salam] initial build failed - waiting for changes"

ENTR_FLAGS=-dc
if [ ! -t 0 ]; then
    ENTR_FLAGS=-dcn
fi

echo "[salam] watching c/src and c/Makefile for changes (Ctrl-C to stop) ..."
while true; do
    find c/src c/Makefile -type f \( -name '*.c' -o -name '*.h' -o -name 'Makefile' \) |
        entr "$ENTR_FLAGS" sh -c 'make -C c OUTDIR=.. -j'"$JOBS"' && echo "[salam] rebuilt ./salam"' ||
        true
done
