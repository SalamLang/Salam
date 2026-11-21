#!/bin/sh
set -u

salam=${1:-}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT INT TERM
export SALAM_STD="$root/std"
cd "$work" || exit 1

printf 'func main:\n    println "\033]0;title\007\302\205\177x" + oops\nend\n' >esc.salam
printf 'func main:\n    println "\330\263\331\204\330\247\331\205" + oops\nend\n' >fa.salam

show() {
    echo "== $*"
    "$salam" "$@" >out.txt 2>&1
    rc=$?
    if grep -q "$(printf '\033')" out.txt; then
        echo "raw escape byte reached the output"
    fi
    grep -vE '^ *[0-9]* *\|? *$' out.txt | sed -e 's/^\[[0-9: -]*\]//'
    echo "exit $rc"
}

show check esc.salam --no-color
show check esc.salam --error-format=json
show check fa.salam --no-color
show check fa.salam --error-format=json
