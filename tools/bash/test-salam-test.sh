#!/bin/sh
set -u

salam=${1:-}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

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
    sed -e "s|$real/||g" -e "s|$work/||g" -e 's/^\[[0-9: -]*\]//' "$work/out.txt"
    echo "exit $rc"
}

cd "$work/en" || exit 1
step test
step test math_test.salam --run=negative
step test lib --format=teamcity
step test lib/strings_test.salam --interp
step test math_test.salam '--run=^test_add$'
step test --run=no_such_test
mkdir broken
printf 'func test_bad():\n    testing.AssertTrue(\nend\n' >broken/bad_test.salam
step test broken
mkdir exits
printf 'func main:\n    ret 3\nend\n' >exits/three_test.salam
step test exits lib/strings_test.salam
cd "$work/fa" || exit 1
step test
cd "$work" || exit 1
mkdir empty
step test empty
