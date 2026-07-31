#!/bin/sh
# Serve the playground (../editor/, relative to the repo's compiler/ dir).

set -eu
. "$(dirname "$0")/lib.sh"
PORT="${1:-3000}"

EDITOR_DIR=../editor

if [ ! -f "$EDITOR_DIR/salam-wa.js" ]; then
    echo "note: $EDITOR_DIR/salam-wa.js is missing the editor loads but the compiler" >&2
    echo "      won't run until you build it with: sh tools/bash/build-wasm.sh" >&2
fi

echo "==> Serving $EDITOR_DIR/ at http://localhost:$PORT/  (Ctrl-C to stop)"
cd "$EDITOR_DIR"
if command -v php >/dev/null 2>&1; then
    exec php -S "0.0.0.0:$PORT"
fi
PY="$(salam_python)"
[ -n "$PY" ] || {
    echo "error: neither php nor python found on PATH" >&2
    exit 1
}
exec "$PY" -m http.server "$PORT"
