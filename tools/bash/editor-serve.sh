#!/bin/sh

set -eu
. "$(dirname "$0")/lib.sh"
PORT="${1:-3000}"

EDITOR_DIR=editor

if ! ls "$EDITOR_DIR"/salam-wa-*.js >/dev/null 2>&1; then
    echo "note: no $EDITOR_DIR/salam-wa-*.js the editor loads but the compiler" >&2
    echo "      won't run until you build it with: sh tools/bash/build-wasm.sh" >&2
fi

echo "==> Serving $EDITOR_DIR/ at http://localhost:$PORT/  (Ctrl-C to stop)"
if command -v php >/dev/null 2>&1; then
    cd "$EDITOR_DIR"
    exec php -S "0.0.0.0:$PORT"
fi
salam_ensure_compiler
exec "$SALAM" serve "$EDITOR_DIR" --host=0.0.0.0 --port="$PORT"
