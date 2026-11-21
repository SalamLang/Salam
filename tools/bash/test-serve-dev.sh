#!/bin/sh

set -u
SALAM="${SALAM:-salam}"
PORT="${1:-49310}"
T=$(mktemp -d)
PID=""
fail=0

# shellcheck disable=SC2329
cleanup() {
    [ -n "$PID" ] && kill "$PID" 2>/dev/null
    rm -rf "$T"
}
trap cleanup EXIT

check() {
    if [ "$2" != "$3" ]; then
        echo "FAIL $1: got '$2', want '$3'"
        fail=1
    else
        echo "ok   $1"
    fi
}

mkdir -p "$T/site" "$T/secret"
cat >"$T/site/app.salam" <<'SAL'
page:
    title = "Dev"
    content = "hello"
end
SAL
echo "outside" >"$T/secret/key.txt"
echo "asset" >"$T/site/logo.txt"
ln -s "$T/secret/key.txt" "$T/site/leak.txt"
echo "stale" >"$T/site/index.html"

"$SALAM" serve "$T/site/app.salam" --port="$PORT" --spa=x.html >"$T/spa.log" 2>&1
check "spa rejected" "$?" "2"

mkdir "$T/dirmain"
mkdir "$T/dirmain/main.salam"
"$SALAM" serve "$T/dirmain" --port="$PORT" >"$T/dm.log" 2>&1 &
DM=$!
sleep 2
code=$(curl -s -o /dev/null -w '%{http_code}' "http://127.0.0.1:$PORT/" 2>/dev/null)
kill "$DM" 2>/dev/null
wait "$DM" 2>/dev/null
check "dir named main.salam is static" "$code" "200"

"$SALAM" serve "$T/site/app.salam" --port="$PORT" >"$T/serve.log" 2>&1 &
PID=$!
i=0
while [ $i -lt 100 ]; do
    curl -s -o /dev/null "http://127.0.0.1:$PORT/__salam_build" 2>/dev/null && break
    sleep 0.3
    i=$((i + 1))
done

id=$(curl -s "http://127.0.0.1:$PORT/__salam_build")
check "build id" "$id" "1"
page=$(curl -s -H 'Accept: text/html' "http://127.0.0.1:$PORT/users/3")
case "$page" in *'var v="1"'*) r=yes ;; *) r=no ;; esac
check "page carries its build id" "$r" "yes"
idx=$(curl -s "http://127.0.0.1:$PORT/index.html")
case "$idx" in *stale*) r=stale ;; *'var v="'*) r=current ;; *) r=other ;; esac
check "index.html is the current build" "$r" "current"
check "asset served" "$(curl -s "http://127.0.0.1:$PORT/logo.txt")" "asset"
code=$(curl -s -o /dev/null -w '%{http_code}' "http://127.0.0.1:$PORT/leak.txt")
check "symlink out of the root refused" "$code" "404"

echo >>"$T/site/app.salam"
i=0
id=1
while [ $i -lt 100 ] && [ "$id" = "1" ]; do
    sleep 0.5
    id=$(curl -s "http://127.0.0.1:$PORT/__salam_build")
    i=$((i + 1))
done
check "rebuild bumps the id" "$id" "2"

kill "$PID" 2>/dev/null
wait "$PID" 2>/dev/null
PID=""
"$SALAM" serve "$T/site/app.salam" --port="$PORT" >"$T/b1.log" 2>&1 &
PID=$!
sleep 3
before=$(find "${TMPDIR:-/tmp}" -maxdepth 1 -name "salam-serve-*" | wc -l)
"$SALAM" serve "$T/site/app.salam" --port="$PORT" >"$T/b2.log" 2>&1
check "busy port fails" "$?" "2"
after=$(find "${TMPDIR:-/tmp}" -maxdepth 1 -name "salam-serve-*" | wc -l)
check "busy port leaves no build directory" "$after" "$before"
kill "$PID" 2>/dev/null
wait "$PID" 2>/dev/null
PID=""

exit "$fail"
