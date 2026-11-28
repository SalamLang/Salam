#!/bin/sh
# A file that sits next to the std folder is not part of the standard library:
# a JavaScript build must still report its ownership errors.

set -u

salam=${1:-}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT INT TERM
mkdir "$work/std"
cp -R "$root/std/." "$work/std/"
cp -R "$root/tests/en/tools/js_std_lints" "$work/app"
export SALAM_STD="$work/std"

if "$salam" build "$work/app/main.salam" --platform=js --output="$work/app.js" --no-color --log-level=error >"$work/build.log" 2>&1; then
    echo "beside std: built without the error"
elif grep -q 'error\[E139\]' "$work/build.log"; then
    echo "beside std: E139"
else
    echo "beside std: build failed"
    sed 's/^/  /' "$work/build.log" | head -20
fi
