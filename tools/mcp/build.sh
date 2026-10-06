#!/usr/bin/env sh
set -eu

root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
out=${1:-"$root/salam-mcp"}
case $out in
/*) ;;
*) out="$(pwd)/$out" ;;
esac

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
case $salam in
/*) ;;
*/*) salam="$(pwd)/$salam" ;;
esac
if ! command -v "$salam" >/dev/null 2>&1; then
    printf 'no salam compiler found: build one with tools/bash/build-selfhost.sh or set SALAM\n' >&2
    exit 1
fi

scratch=$(mktemp -d)
trap 'rm -rf "$scratch"' EXIT
printf 'building salam-mcp with %s\n' "$salam" >&2
(cd "$scratch" && "$salam" build "$root/tools/mcp/main.salam" --output="$out" --log-level=error) >&2
printf 'built %s\n' "$out" >&2
