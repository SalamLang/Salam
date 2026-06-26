#!/bin/sh
# Serve the playground (editor/) over HTTP for development, no rebuild.

set -eu
. "$(dirname "$0")/lib.sh"
PORT="${1:-8080}"
PY="$(salam_python)"
[ -n "$PY" ] || { echo "error: python not found on PATH" >&2; exit 1; }

if [ ! -f ../editor/salam-wa.js ]; then
    echo "note: ../editor/salam-wa.js is missing the editor loads but the compiler" >&2
    echo "      won't run until you build it with: sh tools/bash/build-wasm.sh" >&2
fi

echo "==> Serving ../editor/ at http://localhost:$PORT/  (Ctrl-C to stop)"
cd ../editor && exec "$PY" -m http.server "$PORT"
