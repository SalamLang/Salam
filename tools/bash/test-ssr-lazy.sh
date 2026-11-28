#!/bin/sh
# Builds tests/$SALAM_TEST_LANG/ssr/lazy (en by default) with
# 'salam build --platform=web --ssr' and checks which route texts stay in the
# script and which live only in their pre-rendered page. With playwright-core on
# NODE_PATH and CHROME set, SALAM_TEST_BROWSER=1 also clicks through the build.

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

cp -R "$root/tests/$lang/ssr/lazy" site
"$salam" build site --platform=web --ssr="$work/dist" --no-color --log-level=error --lang="$lang" >build.log 2>&1 || {
    cat build.log
    exit 1
}
echo "build ok"
js=dist/public/index.js

if [ "$lang" = fa ]; then
    set -- "واژه‌های خانه" "واژه‌های درباره" "واژه‌های آغاز" "واژه‌های نصب" "فهرست مستندات" \
        "واژه‌های راهنما" "واژه‌های شمارنده" "واژه‌های گمشده"
    guide="راهنما"
else
    set -- "home page words" "about page words" "intro page words" "setup page words" "docs menu words" \
        "guide page words" "counter page words" "missing page words"
    guide="guide-é"
fi

n=0
for text; do
    n=$((n + 1))
    if grep -qF -- "$text" "$js"; then
        echo "text $n in the script"
    else
        echo "text $n not in the script"
    fi
done

for page in index.html about/index.html docs/intro/index.html docs/setup/index.html "$guide/index.html" counter/index.html 404.html; do
    file="dist/public/$page"
    name=$page
    [ "$page" != "$guide/index.html" ] || name=guide/index.html
    marks=$(grep -o '<!--salam-page:[0-9a-f]*-->' "$file" | wc -l | tr -d ' ')
    echo "$name: $marks marked region(s)"
done
for page in index.html about/index.html docs/intro/index.html docs/setup/index.html; do
    case $page in
    index.html) text=$1 ;;
    about/index.html) text=$2 ;;
    docs/intro/index.html) text=$3 ;;
    *) text=$4 ;;
    esac
    if grep -qF -- "$text" "dist/public/$page"; then
        echo "$page has its text"
    else
        echo "$page lost its text"
    fi
done

PORT=0 SALAM_PUBLIC="$work/dist/public" dist/server >server.log 2>&1 &
pid=$!
url=""
tries=0
while [ -z "$url" ] && [ "$tries" -lt 100 ]; do
    sleep 0.1
    url=$(sed -n 's/^listening on \(http[^ ]*\).*/\1/p' server.log)
    tries=$((tries + 1))
done
curl -s -D - -o /dev/null "$url/docs/intro/" | tr -d '\r' | grep -a '^Location:'

if [ "${SALAM_TEST_BROWSER:-}" = 1 ]; then
    node "$root/tools/js/test-page-lazy.js" "$work/dist/public" "$lang" || exit 1
    node "$root/tools/js/test-page-lazy.js" "$url" "$lang" || exit 1
fi
