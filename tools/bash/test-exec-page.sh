#!/bin/sh

set -u
SALAM="${1:-${SALAM:-salam}}"
PORT=$((49400 + $$ % 400))
T=$(mktemp -d)
PID=""
fail=0

# shellcheck disable=SC2329
cleanup() {
    [ -n "$PID" ] && kill "$PID" 2>/dev/null
    rm -rf "$T"
}
trap cleanup EXIT

has() {
    if grep -qF -- "$3" "$2"; then
        echo "ok   $1"
    else
        echo "FAIL $1: no '$3' in:"
        head -10 "$2"
        fail=1
    fi
}

check() {
    if [ "$2" != "$3" ]; then
        echo "FAIL $1: got '$2', want '$3'"
        fail=1
    else
        echo "ok   $1"
    fi
}

run_exec() {
    out="$1"
    backend="$2"
    shift 2
    SALAM_WINDOW_BACKEND="$backend" SALAM_UI_SCRIPT="$T/app.script" \
        "$SALAM" exec "$@" --no-color --log-level=error >"$out" 2>&1
}

cat >"$T/en.salam" <<'SAL'
component App():
    count := 0
    box:
        heading:
            content = "My Window"
        end
        button:
            id = "inc"
            content = "Clicked " + count
            click = (): count++ println "count is " + count end
        end
    end
end

page:
    title = "My Window"
    App:
    end
end
SAL

cat >"$T/fa.salam" <<'SAL'
بخش برنامه():
    شمار := 0
    جعبه:
        سرتیتر:
            محتوا = "پنجره‌ی من"
        پایان
        دکمه:
            شناسه = "inc"
            محتوا = "کلیک " + شمار
            کلیک = (): شمار++ سرچاپ "count is " + شمار پایان
        پایان
    پایان
پایان

صفحه:
    عنوان = "پنجره‌ی من"
    برنامه:
    پایان
پایان
SAL

printf 'click #inc\nclick #inc\nquit\n' >"$T/app.script"
printf 'func helper:\n    println 1\nend\n' >"$T/plain.salam"

for lang in en fa; do
    run_exec "$T/$lang.native" "" "$T/$lang.salam" --ui=native
    check "$lang native window refused" "$?" "1"

    run_exec "$T/$lang.headless" headless "$T/$lang.salam" --ui=native
    check "$lang headless exit" "$?" "0"
    check "$lang headless output" "$(tr -d '\r' <"$T/$lang.headless" | tr '\n' '|')" "count is 1|count is 2|"

    run_exec "$T/$lang.android" "" "$T/$lang.salam" --platform=android
    check "$lang android refused" "$?" "1"
done

has "en native hint" "$T/en.native" "salam run $T/en.salam --ui=native"
has "en native message" "$T/en.native" "the interpreter cannot open a native window"
has "fa native hint" "$T/fa.native" "salam run $T/fa.salam --ui=native"
has "fa native message" "$T/fa.native" "مفسر نمی‌تواند پنجره‌ی بومی باز کند"
has "en android hint" "$T/en.android" "salam run $T/en.salam --platform=android"
has "fa android message" "$T/fa.android" "مفسر نمی‌تواند صفحه را برای این سکو اجرا کند"

run_exec "$T/js.log" "" "$T/en.salam" --platform=js
check "js refused" "$?" "1"
has "js hint" "$T/js.log" "salam build $T/en.salam --platform=js"

run_exec "$T/bad.log" "" "$T/en.salam" --platform=nosuch
check "unknown platform" "$?" "2"

run_exec "$T/plain.log" "" "$T/plain.salam"
check "no page, no main" "$?" "1"
has "no page, no main message" "$T/plain.log" "no entry point: define a 'main' function"

serve() {
    name="$1"
    file="$2"
    want="$3"
    notice="$4"
    shift 4
    "$SALAM" exec "$file" "$@" --port="$PORT" --no-color >"$T/serve.log" 2>&1 &
    PID=$!
    i=0
    while [ $i -lt 200 ]; do
        curl -s -f -o "$T/page.html" "http://127.0.0.1:$PORT/" 2>/dev/null && break
        kill -0 "$PID" 2>/dev/null || break
        sleep 0.3
        i=$((i + 1))
    done
    has "$name serves the page" "$T/page.html" "$want"
    has "$name says it serves" "$T/serve.log" "$notice"
    kill "$PID" 2>/dev/null
    wait "$PID" 2>/dev/null
    PID=""
    rm -f "$T/page.html"
}

run_exec "$T/timed.log" "" "$T/en.salam" --timeout=3000
check "web under a time limit refused" "$?" "1"
has "web time limit hint" "$T/timed.log" "salam serve $T/en.salam"

serve "en default" "$T/en.salam" "My Window" "the interpreter does not run web pages"
serve "en web ui" "$T/en.salam" "My Window" "the interpreter does not run web pages" --ui=web
serve "fa platform web" "$T/fa.salam" "پنجره‌ی من" "مفسر صفحه‌های وب را اجرا نمی‌کند" --platform=web

mkdir "$T/deskproj" "$T/webproj" "$T/ssrproj"
cp "$T/en.salam" "$T/deskproj/main.salam"
cp "$T/en.salam" "$T/webproj/main.salam"
cp "$T/en.salam" "$T/ssrproj/main.salam"
printf '[package]\nname = "demo"\n[project]\ntype = "dsl"\nplatforms = ["linux", "macos", "windows"]\n' >"$T/deskproj/salam.toml"
printf '[package]\nname = "demo"\n[project]\ntype = "dsl"\nplatforms = ["web"]\n' >"$T/webproj/salam.toml"
printf '[package]\nname = "demo"\n[project]\ntype = "dsl"\nplatforms = ["web"]\nweb = "ssr"\n' >"$T/ssrproj/salam.toml"

run_exec "$T/desk.log" "" "$T/deskproj"
check "desktop project refused" "$?" "1"
has "desktop project hint" "$T/desk.log" "the interpreter cannot open a native window"

run_exec "$T/desk.headless" headless "$T/deskproj"
check "desktop project headless" "$(tr -d '\r' <"$T/desk.headless" | tr '\n' '|')" "count is 1|count is 2|"

case "$(uname -s)" in
Linux | Darwin)
    run_exec "$T/ssr.log" "" "$T/ssrproj"
    check "ssr project refused" "$?" "1"
    has "ssr project message" "$T/ssr.log" "the interpreter cannot run server rendered pages"
    ;;
*)
    echo "ok   ssr project refused"
    echo "ok   ssr project message"
    ;;
esac

serve "web project" "$T/webproj" "My Window" "the interpreter does not run web pages"

exit "$fail"
