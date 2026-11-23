#!/bin/sh
# Opt in memory check for the generated SSR server: builds the test site,
# warms the server up, sends N more requests and compares the resident set
# size. Not part of the default runner.
#   usage: test-ssr-rss.sh [salam] [requests] [limit-kb]

set -u

salam=${1:-}
requests=${2:-2000}
limit=${3:-8192}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

work=$(mktemp -d)
pid=""
cleanup() {
    [ -z "$pid" ] || kill "$pid" 2>/dev/null
    rm -rf "$work"
}
trap cleanup EXIT INT TERM
export SALAM_STD="$root/std"
cd "$work" || exit 1
cp -R "${SITE:-$root/tests/en/ssr/site}" site

"$salam" page build --ssr="$work/dist" site --no-color --log-level=error >build.log 2>&1 || {
    echo "build failed"
    sed 's/^/  /' build.log | head -20
    exit 1
}
PORT=0 HOST=127.0.0.1 ./dist/server >srv.log 2>&1 &
pid=$!
tries=0
until grep -q "listening on" srv.log 2>/dev/null; do
    tries=$((tries + 1))
    [ "$tries" -lt 100 ] || {
        echo "server did not start"
        exit 1
    }
    sleep 0.1
done
port=$(sed -n 's/.*127.0.0.1:\([0-9]*\).*/\1/p' srv.log | head -1)
base="http://127.0.0.1:$port"

burst() {
    n=$1
    i=0
    args=""
    while [ "$i" -lt "$n" ]; do
        for p in / /about /user/7 /total/7 /nope; do
            args="$args -o /dev/null $base$p"
        done
        i=$((i + 5))
    done
    # shellcheck disable=SC2086
    curl -s -m 120 $args
}
rss() {
    sed -n 's/^VmRSS:[ \t]*\([0-9]*\) kB.*/\1/p' "/proc/$pid/status"
}

burst 400
before=$(rss)
burst "$requests"
after=$(rss)
growth=$((after - before))
echo "requests: $requests"
echo "rss before: $before kB"
echo "rss after: $after kB"
echo "growth: $growth kB (limit $limit kB)"
[ "$growth" -le "$limit" ] && echo "ok" || {
    echo "memory grows with requests"
    exit 1
}
