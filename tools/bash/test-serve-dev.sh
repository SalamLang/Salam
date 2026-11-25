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
    curl -s -o /dev/null "http://127.0.0.1:$PORT/__salam_hmr" 2>/dev/null && break
    sleep 0.3
    i=$((i + 1))
done

hello=$(curl -s "http://127.0.0.1:$PORT/__salam_hmr")
case "$hello" in *'"kind":"hello"'*) r=yes ;; *) r=no ;; esac
check "events start with hello" "$r" "yes"
case "$hello" in *'"lang":"en"'*'"dir":"ltr"'*) r=yes ;; *) r=no ;; esac
check "an English project says en and ltr" "$r" "yes"
last=$(printf '%s\n' "$hello" | sed -n 's/^id: //p')
page=$(curl -s -H 'Accept: text/html' "http://127.0.0.1:$PORT/users/3")
case "$page" in *'__salamHmr'*) r=yes ;; *) r=no ;; esac
check "page carries the client" "$r" "yes"
idx=$(curl -s "http://127.0.0.1:$PORT/index.html")
case "$idx" in *stale*) r=stale ;; *'__salamHmr'*) r=current ;; *) r=other ;; esac
check "index.html is the current build" "$r" "current"
check "asset served" "$(curl -s "http://127.0.0.1:$PORT/logo.txt")" "asset"
code=$(curl -s -o /dev/null -w '%{http_code}' "http://127.0.0.1:$PORT/leak.txt")
check "symlink out of the root refused" "$code" "404"

wait_event() {
    curl -s --max-time 30 -H "Last-Event-ID: $1" "http://127.0.0.1:$PORT/__salam_hmr" >"$T/ev.txt" &
    EV=$!
    sleep 1
}

wait_event "$last"
echo >>"$T/site/app.salam"
wait "$EV"
case "$(cat "$T/ev.txt")" in *'"kind":"style"'*) r=style ;; *) r=other ;; esac
check "a change that leaves the markup alone sends style" "$r" "style"
last=$(sed -n 's/^id: //p' "$T/ev.txt")

wait_event "$last"
printf 'page:\n    title = \n' >"$T/site/app.salam"
wait "$EV"
case "$(cat "$T/ev.txt")" in *'"kind":"error"'*) r=error ;; *) r=other ;; esac
check "a broken source sends error" "$r" "error"
last=$(sed -n 's/^id: //p' "$T/ev.txt")
page=$(curl -s -H 'Accept: text/html' "http://127.0.0.1:$PORT/")
case "$page" in *'<title>Dev</title>'*) r=kept ;; *) r=lost ;; esac
check "the last good page is still served" "$r" "kept"

wait_event "$last"
cat >"$T/site/app.salam" <<'SAL'
page:
    title = "Dev"
    content = "hello again"
end
SAL
wait "$EV"
case "$(cat "$T/ev.txt")" in *'"kind":"swap"'*) r=swap ;; *) r=other ;; esac
check "a fixed source recovers with swap" "$r" "swap"
last=$(sed -n 's/^id: //p' "$T/ev.txt")

wait_event "$last"
echo "body{color:red}" >"$T/site/extra.css"
wait "$EV"
case "$(cat "$T/ev.txt")" in *'"kind":"css"'*) r=css ;; *) r=other ;; esac
check "a stylesheet change sends css" "$r" "css"

wait_event "bogus:1"
wait "$EV"
case "$(cat "$T/ev.txt")" in *'"kind":"reload"'*) r=reload ;; *) r=other ;; esac
check "an unknown server instance reloads" "$r" "reload"

hello=$(curl -s "http://127.0.0.1:$PORT/__salam_hmr")
last=$(printf '%s\n' "$hello" | sed -n 's/^id: //p')
n=0
holds=""
while [ $n -lt 12 ]; do
    curl -s --max-time 6 -H "Last-Event-ID: $last" "http://127.0.0.1:$PORT/__salam_hmr" >/dev/null 2>&1 &
    holds="$holds $!"
    n=$((n + 1))
done
sleep 1
t=$(curl -s -o /dev/null -w '%{http_code} %{time_total}' -H 'Accept: text/html' "http://127.0.0.1:$PORT/")
rc=$?
case "$rc $t" in "0 200 0."[0-4]*) r=fast ;; *) r="slow $rc $t" ;; esac
check "pages answer while 12 tabs wait for events" "$r" "fast"
a=$(curl -s -o /dev/null -w '%{http_code} %{time_total}' "http://127.0.0.1:$PORT/logo.txt")
rc=$?
case "$rc $a" in "0 200 0."[0-4]*) r=fast ;; *) r="slow $rc $a" ;; esac
check "assets answer while 12 tabs wait for events" "$r" "fast"
for h in $holds; do
    wait "$h"
done

left=$(find "$T/site" -mindepth 1 -maxdepth 1 -exec basename {} \; | sort | tr '\n' ' ')
check "nothing is written next to the sources" "$left" "app.salam extra.css index.html leak.txt logo.txt "

mine=$(find "${TMPDIR:-/tmp}" -maxdepth 1 -name "salam-serve-$PID-*")
check "the server has a build directory of its own" "$([ -n "$mine" ] && [ -d "$mine" ] && echo yes)" "yes"
kill "$PID" 2>/dev/null
wait "$PID" 2>/dev/null
PID=""
check "stopping the server removes its build directory" "$([ -e "$mine" ] || echo gone)" "gone"
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

mkdir "$T/multi"
printf 'page:\n    title = "A"\n    content = "page a"\nend\n' >"$T/multi/a.salam"
printf 'page:\n    title = "B"\n    content = "page b"\nend\n' >"$T/multi/b.salam"
"$SALAM" serve "$T/multi/a.salam" "$T/multi/b.salam" --port="$PORT" >"$T/multi.log" 2>&1 &
PID=$!
i=0
while [ $i -lt 100 ]; do
    curl -s -o /dev/null "http://127.0.0.1:$PORT/__salam_hmr" 2>/dev/null && break
    sleep 0.3
    i=$((i + 1))
done
check "multi page: second page is served" "$(curl -s "http://127.0.0.1:$PORT/b" | grep -c 'page b')" "1"
check "multi page: unknown page is a 404" "$(curl -s -o /dev/null -w '%{http_code}' -H 'Accept: text/html' "http://127.0.0.1:$PORT/nothing")" "404"
kill "$PID" 2>/dev/null
wait "$PID" 2>/dev/null
PID=""

wait_up() {
    i=0
    while [ $i -lt 100 ]; do
        curl -s -o /dev/null "http://127.0.0.1:$PORT/__salam_hmr" 2>/dev/null && return 0
        sleep 0.3
        i=$((i + 1))
    done
    return 1
}

mkdir "$T/fa"
cat >"$T/fa/app.salam" <<'SAL'
صفحه:
    عنوان = «سلام»
    جعبه:
        محتوا = «سلام دنیا»
    پایان
پایان
SAL
"$SALAM" serve "$T/fa/app.salam" --port="$PORT" >"$T/fa.log" 2>&1 &
PID=$!
wait_up
hello=$(curl -s "http://127.0.0.1:$PORT/__salam_hmr")
case "$hello" in *'"lang":"fa"'*'"dir":"rtl"'*) r=yes ;; *) r=no ;; esac
check "a Persian project says fa and rtl" "$r" "yes"
last=$(printf '%s\n' "$hello" | sed -n 's/^id: //p')
wait_event "$last"
printf 'صفحه:\n    عنوان = \n' >"$T/fa/app.salam"
wait "$EV"
case "$(cat "$T/ev.txt")" in *'"kind":"error"'*'"lang":"fa"'*) r=yes ;; *) r=no ;; esac
check "a Persian build error carries fa" "$r" "yes"
case "$(cat "$T/ev.txt")" in *'انتظار'*) r=yes ;; *) r=no ;; esac
check "the diagnostics are Persian" "$r" "yes"
page=$(curl -s -H 'Accept: text/html' "http://127.0.0.1:$PORT/")
case "$page" in *'<html lang="fa" dir="rtl"'*|*'سلام دنیا'*) r=yes ;; *) r=no ;; esac
check "the page keeps its language" "$r" "yes"
kill "$PID" 2>/dev/null
wait "$PID" 2>/dev/null
PID=""

cat >"$T/fa/app.salam" <<'SAL'
صفحه:
    عنوان = «سلام»
    جعبه:
        محتوا = «سلام دنیا»
    پایان
پایان
SAL
"$SALAM" serve "$T/fa/app.salam" --error-lang=en --port="$PORT" >"$T/fa2.log" 2>&1 &
PID=$!
wait_up
hello=$(curl -s "http://127.0.0.1:$PORT/__salam_hmr")
case "$hello" in *'"lang":"en"'*'"dir":"ltr"'*) r=yes ;; *) r=no ;; esac
check "--error-lang=en overrides a Persian source" "$r" "yes"
kill "$PID" 2>/dev/null
wait "$PID" 2>/dev/null
PID=""

mkdir "$T/built"
printf '<!doctype html><html data-lang="en" dir="rtl" lang = "fa-IR"><head><meta charset="utf-8"></head><body>x</body></html>' >"$T/built/index.html"
"$SALAM" serve "$T/built" --watch --port="$PORT" >"$T/built.log" 2>&1 &
PID=$!
wait_up
hello=$(curl -s "http://127.0.0.1:$PORT/__salam_hmr")
case "$hello" in *'"lang":"fa"'*) r=yes ;; *) r=no ;; esac
check "a built folder takes the language of its page" "$r" "yes"
kill "$PID" 2>/dev/null
wait "$PID" 2>/dev/null
PID=""

exit "$fail"
