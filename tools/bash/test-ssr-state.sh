#!/bin/sh
# Builds tests/$SALAM_TEST_LANG/ssr/state (en by default) with
# 'salam build --platform=web --ssr' and runs tools/js/test-ssr-state.js, which
# starts the server and checks when each route's server functions are called
# (CHROME, and playwright-core on NODE_PATH).

set -u

salam=${1:-}
lang=${SALAM_TEST_LANG:-en}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT INT TERM
export SALAM_STD="$root/std"
cd "$work" || exit 1

cp -R "$root/tests/$lang/ssr/state" site
"$salam" build site --platform=web --ssr="$work/dist" --no-color --log-level=error --lang="$lang" >build.log 2>&1 || {
    cat build.log
    exit 1
}
SALAM_TEST_LANG="$lang" node "$root/tools/js/test-ssr-state.js" "$work/dist"
