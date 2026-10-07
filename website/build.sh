#!/bin/sh

set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
. "$root/tools/bash/lib.sh"
cd "$root"
salam_ensure_compiler
case "$SALAM" in
/*) ;;
*/*) SALAM="$root/$SALAM" ;;
esac

"$SALAM" run tools/salam/check-fa-names.salam --no-color --log-level=error

export SALAM
jobs=$(nproc 2>/dev/null || echo 4)
fail=0
printf '%s\0' website/content/*/examples/*.salam website/content/*/std-examples/*/*.salam |
    xargs -0 -P "$jobs" -n 1 sh website/check-example.sh || fail=1
[ "$fail" -eq 0 ] || {
    echo "website: example output does not match its .out file" >&2
    exit 1
}

mkdir -p website/dist/api
for page in website/content/*/std/*.txt; do
    [ -f "$page" ] || continue
    pkg=$(sed -n 's/^pkg\.en = //p' "$page")
    slug=$(printf '%s' "$pkg" | tr '/' '-')
    src="std/$pkg"
    if grep -q '^pkg\.flat = true' "$page"; then
        src=$(mktemp -d)
        cp "std/$pkg"/*.salam "$src/"
    fi
    if ! "$SALAM" doc "$src" --output="website/dist/api/$slug.json" >/dev/null; then
        echo "website: salam doc failed for std/$pkg" >&2
        exit 1
    fi
done

"$SALAM" run website/build.salam --no-color --log-level=error "$@"
