#!/bin/sh
# Builds a small pages project with 'salam build --platform=web --ssr', starts the
# generated server on a free port and checks routes, status codes, redirects,
# the rendered HTML and the escaping of hostile input.

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

echo "== build"
if "$salam" build site --platform=web --ssr="$work/dist" --no-color --log-level=error --lang="$lang" >build.log 2>&1; then
    echo "build ok"
else
    echo "build failed"
    sed 's/^/  /' build.log | head -30
    exit 1
fi
echo "== lock"
mkdir dist.lock
echo $$ >dist.lock/pid
if "$salam" build site --platform=web --ssr="$work/dist" --no-color --log-level=error --lang="$lang" >lock.log 2>&1; then
    echo "second build ran"
else
    grep -q "$work/dist" lock.log && echo "second build refused"
fi
[ -d dist.lock ] && echo "lock kept for its owner"
rm -rf dist.lock
[ -d dist.tmp ] && echo "staging left behind" || echo "no staging left"
[ -x dist/server ] && echo "server written"
[ -f dist/public/.shell.html ] && echo "shell written"
ls dist/public | grep -c '\.js$' | sed 's/^/js files: /'
ls dist/public | grep -c '\.css$' | sed 's/^/css files: /'
[ -f dist/public/about/index.html ] && echo "prerendered about"
[ -f dist/public/user/7/index.html ] || echo "no prerender for a parameter route"

echo "== start"
PORT=0 HOST=127.0.0.1 ./dist/server >srv.log 2>&1 &
pid=$!
tries=0
until grep -q "listening on" srv.log 2>/dev/null; do
    tries=$((tries + 1))
    [ "$tries" -lt 100 ] || {
        echo "server did not start"
        cat srv.log
        exit 1
    }
    sleep 0.1
done
port=$(sed -n 's/.*127.0.0.1:\([0-9]*\).*/\1/p' srv.log | head -1)
base="http://127.0.0.1:$port"
echo "server up"

get() {
    curl -s -m 10 -o body.html -D head.txt -w '%{http_code}' "$base$1"
}
has() {
    grep -qF -- "$1" body.html && echo "  has $2" || echo "  MISSING $2"
}
hasnot() {
    grep -qF -- "$1" body.html && echo "  UNEXPECTED $2" || echo "  lacks $2"
}
title() {
    sed -n 's/.*<title>\([^<]*\)<\/title>.*/\1/p' body.html | head -1
}

case "$lang" in
en)
    t_home="Home"
    t_about="About us"
    w_home="Welcome"
    w_about="We sell things."
    w_user="User 7"
    w_missing="Nothing at /nope"
    w_total="Total 21"
    ;;
fa)
    t_home="خانه"
    t_about="درباره ما"
    w_home="خوش آمدید"
    w_about="ما چیز می‌فروشیم."
    w_user="کاربر 7"
    w_missing="چیزی در /nope نیست"
    w_total="مجموع 21"
    ;;
esac

echo "== GET /"
echo "status $(get /)"
echo "  title: $(title)"
has "$w_home" welcome
has 'id="__salam"' state
has '"status":200' status-in-state

echo "== GET /about"
echo "status $(get /about)"
echo "  title: $(title)"
has "$w_about" body

echo "== GET /user/7"
echo "status $(get /user/7)"
echo "  title: $(title)"
has "$w_user" heading
has '"pattern":"/user/:id"' pattern-in-state

echo "== GET /user/abc"
echo "status $(get /user/abc)"

echo "== GET /total/7"
echo "status $(get /total/7)"
has "$w_total" body
has '"calls":{"Price:' call-in-state
has ':0":21' call-value

echo "== GET /admin"
echo "status $(get /admin)"
has "$(printf '%s' "$w_missing" | sed 's#/nope#/admin#')" missing-text

echo "== GET /admin/users"
echo "status $(get /admin/users)"
has "$(printf '%s' "$w_missing" | sed 's#/nope#/admin/users#')" missing-text

echo "== GET /nope"
echo "status $(get /nope)"
has "$w_missing" missing-text
has '"status":404' status-in-state

echo "== hostile path"
echo "status $(get '/%3C/script%3E%3Cscript%3Ealert(1)%3C/script%3E?q=%3Cb%3E')"
hasnot '</script><script>alert' raw-script
has '&lt;/script&gt;&lt;script&gt;alert(1)' escaped-body
has '"path":"/\u003c/script' escaped-state
hasnot "<b>" raw-query

echo "== hostile quote in an attribute"
echo "status $(get '/%22onmouseover=%22x')"
has 'href="/&quot;onmouseover=&quot;x"' escaped-href
hasnot 'href="/"onmouseover' raw-href
has '"path":"/\"onmouseover=\"x"' json-escaped-quote

echo "== hostile line break"
echo "status $(get '/x%0d%0aSet-Cookie:%20a=b')"
grep -qi '^set-cookie' head.txt && echo "  UNEXPECTED header injection" || echo "  no header injection"

echo "== static"
js=$(ls dist/public | grep '\.js$' | head -1)
echo "status $(get "/$js")"
echo "status $(get /.shell.html)"
echo "status $(get /..%2f..%2fetc/passwd)"

echo "== server html equals the client build"
get /about >/dev/null
sed 's#<script type="application/json" id="__salam">[^<]*</script>##' body.html >served.html
if cmp -s served.html dist/public/about/index.html; then
    echo "about: same"
else
    echo "about: differs"
    diff served.html dist/public/about/index.html | head -10
fi
get / >/dev/null
sed 's#<script type="application/json" id="__salam">[^<]*</script>##; s#<title>[^<]*</title>##' body.html >served.html
sed 's#<title>[^<]*</title>##' dist/public/index.html >static.html
if cmp -s served.html static.html; then
    echo "home: same"
else
    echo "home: differs"
    diff served.html static.html | head -10
fi
