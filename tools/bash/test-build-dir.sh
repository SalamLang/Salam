#!/bin/sh
# Tests that builds of different projects started from one directory at the
# same time do not overwrite each other's generated files.

set -u

salam=${1:-}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT INT TERM
export SALAM_STD="$root/std"
cd "$work" || exit 1

cc=${CC:-}
if [ -z "$cc" ]; then
    for c in cc gcc clang; do
        if command -v "$c" >/dev/null 2>&1; then
            cc=$c
            break
        fi
    done
fi

for p in a b c d; do
    mkdir "$p"
    printf 'func main:\n    println "I am %s"\nend\n' "$p" >"$p/main.salam"
done

echo "== concurrent builds in one directory"
pids=""
for p in a b c d; do
    "$salam" build "$p/main.salam" --backend=c --cc="$cc" -o "out_$p" --no-color >"log_$p" 2>&1 &
    pids="$pids $!"
done
for pid in $pids; do
    wait "$pid" || echo "build failed"
done
for p in a b c d; do
    echo "$p: $(./out_$p 2>&1)"
done

echo "== explicit SALAM_BUILD_DIR"
SALAM_BUILD_DIR="$work/custom" "$salam" build a/main.salam --backend=c --cc="$cc" -o out_custom --no-color >log_custom 2>&1
echo "custom: $(./out_custom 2>&1)"
if ls custom/salam_mod_*.c >/dev/null 2>&1; then
    echo "custom dir used"
else
    echo "custom dir unused"
fi
