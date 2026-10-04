#!/bin/sh
set -eu

root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$root"

salam=${SALAM:-./salam}
if [ ! -x "$salam" ]; then
    printf 'salam is not built; skipping the message table check.\n' >&2
    exit 0
fi

exec "$salam" run tools/salam/check-i18n.salam --no-color --log-level=error
