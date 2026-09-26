#!/usr/bin/env bash
set -euo pipefail

here="$(cd "$(dirname "$0")" && pwd)"
salam="${SALAM:-$here/../../../../salam}"
[ -x "$salam" ] || salam="salam"

mkdir -p "$here/build"
"$salam" build "$here/main.salam" --target=wasm32-wasi --release --output="$here/build/app.wasm" --log-level=error --no-color
