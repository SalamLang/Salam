#!/bin/sh
# Fails the commit when any hardcoded version disagrees with VERSION.
#
# The site list lives in tools/salam/update-version.salam, so the hook, the
# updater and the CI job all read one table and none of them can drift from
# the others. This wrapper only finds a compiler to run it with.
set -eu

root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$root"

# A released salam on PATH, the repo's own build, or $SALAM. Without one the
# hook steps aside rather than failing the commit: a contributor who has not
# built the compiler yet still gets every other hook, and the CI job
# (lint-version-sync.yml) installs a compiler and runs the same check, so
# nothing reaches main unchecked.
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
