#!/bin/sh
# Live-rebuild loop for the Salam compiler inside the `dev` container.

set -eu

JOBS="$(nproc 2>/dev/null || echo 1)"

echo "[salam] initial build (make OUTDIR=. -j$JOBS) ..."
make OUTDIR=. -j"$JOBS" || echo "[salam] initial build failed - waiting for changes"

ENTR_FLAGS=-dc
if [ ! -t 0 ]; then
    ENTR_FLAGS=-dcn
fi

echo "[salam] watching src/ and Makefile for changes (Ctrl-C to stop) ..."
while true; do
    find src Makefile -type f \( -name '*.c' -o -name '*.h' -o -name 'Makefile' \) |
        entr "$ENTR_FLAGS" sh -c 'make OUTDIR=. -j'"$JOBS"' && echo "[salam] rebuilt ./salam"' ||
        true
done
