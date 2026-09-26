#!/usr/bin/env bash
set -euo pipefail

here="$(cd "$(dirname "$0")" && pwd)"
repo="$(cd "$here/../../../.." && pwd)"
salam="${SALAM:-$repo/salam}"

mkdir -p "$here/build"
"$salam" build "$here/main.salam" --target=wasm32-wasi --release --output="$here/build/app.wasm" --log-level=error --no-color

echo "built $here/build/app.wasm ($(wc -c <"$here/build/app.wasm" | tr -d ' ') bytes)"
