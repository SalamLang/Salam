#!/bin/sh
set -eu

root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$root"

names=$(sed -n 's/^k_pm_official := \[\(.*\)\]$/\1/p' compiler/pkgmgr/official.salam | tr -d '" ' | tr ',' '\n')
[ -n "$names" ] || {
    echo "cannot read the official package list" >&2
    exit 1
}

pattern=$(printf '%s\n' "$names" | sed 's/\./\\./g' | paste -sd '|' -)
hits=$(grep -rnE "^[[:space:]]*import[[:space:]]+($pattern)([[:space:]]|$)" --include='*.salam' std compiler || true)
if [ -n "$hits" ]; then
    echo "std/ and compiler/ must not import official packages; they do not ship with Salam:" >&2
    printf '%s\n' "$hits" >&2
    exit 1
fi
echo "std and compiler import no official package"
