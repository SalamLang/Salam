#!/bin/sh
set -u

salam=${1:-}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT INT TERM
export SALAM_STD="$root/std"

(cd "$work" && "$salam" build --backend=c "$root/tools/salam/migrate-style.salam" --output="$work/migrate" --no-color --log-level=error >"$work/build.log" 2>&1)
if [ ! -x "$work/migrate" ]; then
    echo "migrate-style did not build"
    cat "$work/build.log"
    exit 1
fi

cd "$root" || exit 1
for lang in en fa; do
    for f in tests/$lang/tools/migrate_*.salam; do
        [ -e "$f" ] || continue
        echo "== $f"
        "$work/migrate" "$f" 2>&1
        echo "exit $?"
    done
done
