#!/bin/sh
# A two file program whose included file has no 'package' line must build on
# every backend, and equal private names in the two files must stay apart.

set -u

salam=${1:-}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT INT TERM
export SALAM_STD="$root/std"
cp -R "$root/tests/en/project/include_unit" "$work/app"
cd "$work/app" || exit 1

expect() {
    got=$(tr -d '\r' <out.txt | tr '\n' ' ')
    if [ "$got" = "8 3 " ]; then
        return 0
    fi
    echo "$1: wrong output: $got"
    return 1
}

for backend in c llvm native; do
    if [ "$backend" = native ] && [ "$(uname -s)-$(uname -m)" != Linux-x86_64 ]; then
        continue
    fi
    if "$salam" build main.salam --backend="$backend" --output="app_$backend" --no-color --log-level=error >build.log 2>&1; then
        "./app_$backend" >out.txt 2>&1
        if expect "$backend" && [ "$backend" != native ]; then
            echo "$backend: ok"
        fi
    else
        echo "$backend: build failed"
        sed 's/^/  /' build.log | head -20
    fi
done

"$salam" run main.salam --interp --no-color --log-level=error >out.txt 2>&1
expect interp && echo "interp: ok"

if command -v node >/dev/null 2>&1; then
    if "$salam" build main.salam --platform=js --output=app.js --no-color --log-level=error >build.log 2>&1; then
        node app.js >out.txt 2>&1
        expect js
    else
        echo "js: build failed"
        sed 's/^/  /' build.log | head -20
    fi
fi

if "$salam" check main.salam --backend=c --no-color --log-level=error >out.txt 2>&1; then
    echo "check: ok"
else
    echo "check: failed"
    sed 's/^/  /' out.txt
fi
