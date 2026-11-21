#!/bin/sh
# Tests that `salam build` reuses an up to date executable only when the
# sources and every flag that changes the output are unchanged.

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

prog() {
    printf 'func main:\n    println "%s"\nend\n' "$1" >q.salam
}

build() {
    "$salam" build q.salam --backend=c --cc="$cc" --log-level=info --no-color "$@" >build.log 2>&1
}

echo "== edit in the same second"
prog v1
build || {
    echo "build failed"
    cat build.log
    exit 1
}
echo "first: $(./q)"
prog v2
build || {
    echo "build failed"
    cat build.log
    exit 1
}
echo "second: $(./q)"

echo "== unchanged project"
sleep 2
build || {
    echo "build failed"
    cat build.log
    exit 1
}
build || {
    echo "build failed"
    cat build.log
    exit 1
}
if grep -q "up to date" build.log; then
    echo "repeat: up to date"
else
    echo "repeat: rebuilt"
fi

echo "== flags"
build --protect || {
    echo "build failed"
    cat build.log
    exit 1
}
if grep -q "up to date" build.log; then
    echo "protect: skipped"
else
    echo "protect: rebuilt"
fi
build --protect || {
    echo "build failed"
    cat build.log
    exit 1
}
if grep -q "up to date" build.log; then
    echo "protect again: up to date"
else
    echo "protect again: rebuilt"
fi
