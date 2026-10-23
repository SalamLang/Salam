#!/bin/sh
set -u

salam=${1:-}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /*) ;; *) salam="$root/$salam" ;; esac

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT INT TERM
real=$(cd "$work" && pwd -P)
export SALAM_STD="$root/std"
mkdir -p "$work/en" "$work/fa"
cp -R "$root/tests/en/testcmd/." "$work/en/"
cp -R "$root/tests/fa/testcmd/." "$work/fa/"

step() {
    echo "== $*"
    "$salam" "$@" >"$work/out.txt" 2>&1
    rc=$?
    sed -e "s|$real/||g" -e "s|$work/||g" "$work/out.txt"
    echo "exit $rc"
}

cd "$work/en" || exit 1
step test
step test math_test.salam --run=negative
step test lib --format=teamcity
step test lib/strings_test.salam --interp
cd "$work/fa" || exit 1
step test
cd "$work" || exit 1
mkdir empty
step test empty
