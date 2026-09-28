#!/bin/sh

set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
. "$root/tools/bash/lib.sh"
cd "$root"
salam_ensure_compiler

fail=0
for src in website/content/*/examples/*.salam; do
    want="${src%.salam}.out"
    if [ ! -f "$want" ]; then
        echo "MISSING $want"
        fail=1
        continue
    fi
    if ! got=$("$SALAM" run "$src" --no-color --log-level=error 2>&1); then
        echo "FAIL $src"
        printf '%s\n' "$got"
        fail=1
        continue
    fi
    if [ "$got" != "$(cat "$want")" ]; then
        echo "DIFF $src"
        printf '%s\n' "$got" | diff "$want" - || true
        fail=1
        continue
    fi
    echo "ok   $src"
done
[ "$fail" -eq 0 ] || {
    echo "website: example output does not match its .out file" >&2
    exit 1
}

"$SALAM" run website/build.salam --no-color --log-level=error "$@"
