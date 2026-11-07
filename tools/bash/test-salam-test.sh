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
mkdir -p expect
printf '// EXPECT: hello\n// EXPECT-NOT: goodbye\n// EXPECT-EXIT: 0\nfunc main:\n    println "hello"\nend\n' >expect/out.test.salam
step test expect/out.test.salam
printf '// EXPECT: never printed\nfunc main:\n    println "hello"\nend\n' >expect/bad.test.salam
step test expect/bad.test.salam
printf '// SKIP: not ready\nfunc main:\nend\n' >expect/skip.test.salam
step test expect/skip.test.salam
printf '// EXPECT-CODE: E059\nfunc main:\n    unused := 1\nend\n' >expect/err.test.salam
step test expect/err.test.salam
printf 'func main:\n    // skip: a comment in the body is not a directive\n    println "ok"\nend\n' >expect/bodycomment.test.salam
step test expect/bodycomment.test.salam
printf '// EXPECT-EXIT: nope\nfunc main:\nend\n' >expect/badnum.test.salam
step test expect/badnum.test.salam
"$salam" test expect/out.test.salam --report="$work/report.tsv" >/dev/null 2>&1
printf 'report: '
cat "$work/report.tsv"
step test expect --list
step test expect/out.test.salam --jobs=1
cd "$work/fa" || exit 1
step test
mkdir -p exp
printf '// انتظار: سلام\nfunc main:\n    println "سلام"\nend\n' >exp/fa_expect.test.salam
step test exp
cd "$work" || exit 1
mkdir empty
step test empty
