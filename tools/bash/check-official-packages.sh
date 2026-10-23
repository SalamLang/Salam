#!/bin/sh
set -eu

root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$root"

list=$(sed -n 's/^k_pm_official := \[\(.*\)\]$/\1/p' compiler/pkgmgr/official.salam | tr -d '" ' | tr ',' '\n' | sort)
found=$(find packages -name '*.salam' | while read -r f; do
    d=$(dirname "$f")
    [ "$(basename "$f" .salam)" = "$(basename "$d")" ] && printf '%s\n' "${d#packages/}" | tr '/' '.'
done | sort -u)

if [ "$list" != "$found" ]; then
    echo "compiler/pkgmgr/official.salam and packages/ disagree:" >&2
    echo "  listed:  $(printf '%s' "$list" | tr '\n' ' ')" >&2
    echo "  present: $(printf '%s' "$found" | tr '\n' ' ')" >&2
    exit 1
fi
echo "official packages: $(printf '%s\n' "$list" | wc -l) listed, all present"
