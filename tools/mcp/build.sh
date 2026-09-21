#!/usr/bin/env sh
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
