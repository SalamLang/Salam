#!/bin/sh
# Serve the playground (web/) over HTTP for development — no rebuild.

set -eu
. "$(dirname "$0")/lib.sh"
PORT="${1:-8080}"
PY="$(salam_python)"
[ -n "$PY" ] || { echo "error: python not found on PATH" >&2; exit 1; }
if [ ! -f web/salam-wa.js ]; then
    echo "note: web/salam-wa.js is missing — the editor loads but the compiler" >&2
    echo "      won't run until you build it with: sh tools/build-wasm.sh" >&2
fi
echo "==> Serving web/ at http://localhost:$PORT/  (Ctrl-C to stop)"
cd web && exec "$PY" -m http.server "$PORT"
