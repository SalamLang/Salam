#!/usr/bin/env sh
# Builds the salam-mcp server with the Salam compiler in this checkout.
#
# Usage:
#   tools/mcp/build.sh [output-path]
#
# The compiler used is, in order: $SALAM, ./salam, ./salam.exe, then whatever
# `salam` resolves to on PATH. Building with the checkout's own compiler
# matters - an older installed salam parses some flags differently and the
# resulting server fails in confusing ways.
set -eu

root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
out=${1:-"$root/salam-mcp"}

pick_compiler() {
    if [ -n "${SALAM:-}" ]; then
        printf '%s' "$SALAM"
        return
    fi
    if [ -x "$root/salam" ]; then
        printf '%s' "$root/salam"
        return
    fi
    if [ -x "$root/salam.exe" ]; then
        printf '%s' "$root/salam.exe"
        return
    fi
    printf 'salam'
}

salam=$(pick_compiler)
printf 'building salam-mcp with %s\n' "$salam" >&2
"$salam" build "$root/tools/mcp/main.salam" --output="$out"
printf 'built %s\n' "$out" >&2
