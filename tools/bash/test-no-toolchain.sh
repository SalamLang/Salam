#!/bin/sh
# A build on a system without any C compiler prints one message naming what to
# install, and no shell "not found" noise.

set -u

salam=${1:-}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

work=$(mktemp -d)
export SALAM_STD="$root/std"
cd "$work" || exit 1
mkdir empty
printf 'func main:\n    println "hi"\nend\n' >hello.salam

PATH="$work/empty" "$salam" build hello.salam --backend=c --output=hello --no-color --log-level=error >out.txt 2>&1
echo "exit $?"
echo "messages: $(grep -c 'missing the C toolchain' out.txt)"
echo "error lines: $(grep -c '\[ERROR\]' out.txt)"
echo "noise lines: $(grep -c ': not found\|falling back' out.txt)"
grep -q 'apt install gcc libc6-dev' out.txt && echo "names the packages"
