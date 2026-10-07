#!/bin/sh

set -u
root=$(cd "$(dirname "$0")/.." && pwd)
src=$1
: "${SALAM_STD:=$root/std}"
export SALAM_STD
want="$root/${src%.salam}.out"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
cd "$tmp" || exit 1

if [ ! -f "$want" ]; then
    if grep -q "^چیدمان:\|^layout:" "$root/$src"; then
        cp "$root/$src" .
        if "$SALAM" layout build "$(basename "$src")" --inline --no-color --log-level=error >/dev/null 2>&1; then
            echo "ok   $src (layout build)"
            exit 0
        fi
        echo "FAIL $src (layout build)"
        exit 1
    fi
    if "$SALAM" inspect --emit-ast "$root/$src" --no-color --log-level=error >/dev/null 2>&1; then
        echo "ok   $src (checked, not run)"
        exit 0
    fi
    echo "FAIL $src (does not compile)"
    exit 1
fi

if ! got=$("$SALAM" run "$root/$src" --no-color --log-level=error 2>&1 </dev/null); then
    printf 'FAIL %s\n%s\n' "$src" "$got"
    exit 1
fi
got=$(printf '%s' "$got" | tr -d '\r')
if [ "$got" != "$(tr -d '\r' <"$want")" ]; then
    printf 'DIFF %s\n%s\n' "$src" "$(printf '%s\n' "$got" | diff "$want" - || true)"
    exit 1
fi
echo "ok   $src"
