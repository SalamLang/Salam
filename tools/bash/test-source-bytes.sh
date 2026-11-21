#!/bin/sh
set -u

salam=${1:-}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

work=$(mktemp -d)
trap 'chmod -R u+rw "$work" 2>/dev/null; rm -rf "$work"' EXIT INT TERM
export SALAM_STD="$root/std"
cd "$work" || exit 1

printf '\357\273\277func main:\n    println "bom ok"\nend\n' >bom.salam
printf 'func main:\n    println "a"\nend\n\000 garbage ((((\n' >nul.salam
printf 'func main:\n    println "a\377\376b"\nend\n' >badutf.salam
printf 'func main:\n    println "\330\263\331\204\330\247\331\205\342\200\214\330\257\331\206\333\214\330\247"\nend\n' >persian.salam
printf 'func main:\n    println "hidden"\nend\n' >noperm.salam

step() {
    echo "== $*"
    "$salam" "$@" --no-color >out.txt 2>&1
    rc=$?
    grep -vE '^ *[0-9]* *\|' out.txt | sed -e 's/^\[[0-9: -]*\]//'
    echo "exit $rc"
}

step run bom.salam
step check nul.salam
step check badutf.salam
step run persian.salam

chmod 000 noperm.salam
echo "== run noperm.salam"
if [ -r noperm.salam ]; then
    echo "cannot read source file 'noperm.salam': permission denied or the file cannot be opened"
    echo "build aborted: errors in source (noperm.salam)"
    echo "exit 1"
else
    "$salam" run noperm.salam --no-color >out.txt 2>&1
    rc=$?
    sed -e 's/^\[[0-9: -]*\]\[[A-Z]*\]\[[A-Z]*\] //' out.txt
    echo "exit $rc"
fi
