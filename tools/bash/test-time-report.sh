#!/bin/sh
# Tests that --time-report prints no counters it cannot fill and that the help
# text names the real default C compiler order.

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

printf 'func main:\n    println "hi"\nend\n' >t.salam

echo "== time report"
if ! "$salam" build t.salam --backend=c --cc="$cc" --force --time-report --no-color >report.txt 2>&1; then
    echo "build failed"
    cat report.txt
    exit 1
fi
for word in "ast nodes" "arena" "peak rss 0 B"; do
    if grep -q "$word" report.txt; then
        echo "report prints: $word"
    fi
done
if grep -q "tokens  *[1-9]" report.txt; then
    echo "tokens counted"
fi

echo "== help"
"$salam" --help 2>&1 | grep -q "clang" && echo "help names clang"
"$salam" --help 2>&1 | grep -q "default: tcc" && echo "help still says tcc"
exit 0
