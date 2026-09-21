#!/bin/sh
set -eu

root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$root"

salam=${SALAM:-}
if [ -z "$salam" ] && command -v salam >/dev/null 2>&1; then
    salam=$(command -v salam)
fi
if [ -z "$salam" ] && [ -x "$root/salam" ]; then
    salam="$root/salam"
fi
if [ -z "$salam" ]; then
    printf 'no salam compiler found; skipping the version check.\n' >&2
    printf 'Build one (tools/bash/build-selfhost.sh) or set $SALAM to enable it.\n' >&2
    exit 0
fi

exec "$salam" run tools/salam/update-version.salam check
