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

fail=0
for src in website/content/*/examples/*.salam; do
    want="${src%.salam}.out"
    if [ ! -f "$want" ]; then
        if grep -q "^چیدمان:\|^layout:" "$src"; then
            tmp=$(mktemp -d)
            cp "$src" "$tmp/"
            if (cd "$tmp" && "$SALAM" layout build "$(basename "$src")" --inline --no-color --log-level=error >/dev/null 2>&1); then
                echo "ok   $src (layout build)"
            else
                echo "FAIL $src (layout build)"
                fail=1
            fi
            rm -rf "$tmp"
        elif "$SALAM" inspect --emit-ast "$src" --no-color --log-level=error >/dev/null 2>&1; then
            echo "ok   $src (checked, not run)"
        else
            echo "FAIL $src (does not compile)"
            fail=1
        fi
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
for src in website/content/*/std-examples/*/*.salam; do
    [ -f "$src" ] || continue
    want="${src%.salam}.out"
    tmp=$(mktemp -d)
    if [ ! -f "$want" ]; then
        if (cd "$tmp" && "$SALAM" inspect --emit-ast "$root/$src" --no-color --log-level=error >/dev/null 2>&1); then
            echo "ok   $src (checked, not run)"
        else
            echo "FAIL $src (does not compile)"
            fail=1
        fi
        rm -rf "$tmp"
        continue
    fi
    if ! got=$(cd "$tmp" && "$SALAM" run "$root/$src" --no-color --log-level=error 2>&1 </dev/null); then
        echo "FAIL $src"
        printf '%s\n' "$got"
        fail=1
    elif [ "$(printf '%s' "$got" | tr -d '\r')" != "$(tr -d '\r' <"$want")" ]; then
        echo "DIFF $src"
        printf '%s\n' "$got" | tr -d '\r' | diff "$want" - || true
        fail=1
    else
        echo "ok   $src"
    fi
    rm -rf "$tmp"
done
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
