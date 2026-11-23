#!/bin/sh
# Builds tests/$SALAM_TEST_LANG/ssr/site (en by default) with 'salam page build --ssr', starts the server and
# runs tools/js/test-ssr-hydrate.js against it in a browser (CHROME, and
# playwright-core on NODE_PATH).

set -u

salam=${1:-}
lang=${SALAM_TEST_LANG:-en}
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

cp -R "$root/tests/$lang/ssr/site" site
"$salam" page build --ssr="$work/dist" site --no-color --log-level=error --lang="$lang" >build.log 2>&1 || {
    cat build.log
    exit 1
}
PORT=0 HOST=127.0.0.1 ./dist/server >srv.log 2>&1 &
pid=$!
tries=0
until grep -q "listening on" srv.log 2>/dev/null; do
    tries=$((tries + 1))
    [ "$tries" -lt 100 ] || {
        cat srv.log
        exit 1
    }
    sleep 0.1
done
port=$(sed -n 's/.*127.0.0.1:\([0-9]*\).*/\1/p' srv.log | head -1)
case "$lang" in fa) SALAM_TEST_COUNT_WORD="تعداد" ;; *) SALAM_TEST_COUNT_WORD="Count" ;; esac
export SALAM_TEST_COUNT_WORD
node "$root/tools/js/test-ssr-hydrate.js" "http://127.0.0.1:$port" "$work/dist/public"
