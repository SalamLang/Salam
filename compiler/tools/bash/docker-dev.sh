#!/bin/sh
# Live-rebuild loop for the self-hosted Salam compiler inside the `dev`
# container. Runs from the repo root (docker-compose.yml now mounts the
# whole repo at /app, not just compiler/, since std/ and tests/ live
# alongside compiler/ post-restructure).

set -eu

echo "[salam] initial build (self-hosted: compiler/salam building compiler/main.salam) ..."
sh compiler/tools/bash/build-compiler.sh || echo "[salam] initial build failed - waiting for changes"

ENTR_FLAGS=-dc
if [ ! -t 0 ]; then
    ENTR_FLAGS=-dcn
fi

echo "[salam] watching compiler/*.salam for changes (Ctrl-C to stop) ..."
while true; do
    find compiler -maxdepth 1 -type f -name '*.salam' |
        entr "$ENTR_FLAGS" sh -c 'sh compiler/tools/bash/build-compiler.sh && echo "[salam] rebuilt compiler/salam"' ||
        true
done
