#!/bin/sh
# Builds a std/gui/webview program for Windows against a mock WebView2Loader.dll
# (std/gui/webview/native/mock/webview2_mock.c) and runs it under wine, which checks
# the COM calling convention both ways: the program's vtable calls, and the
# handlers the mock calls back, which must pop their own arguments on i686.
#
# Needs wine on PATH, Xvfb (wine has no window driver without a display) and a
# mingw-w64 compiler for each target, <arch>-w64-mingw32-gcc from mingw-w64 or
# llvm-mingw. NATIVE_WEBVIEW_TRIPLES lists the targets, by default i686 and x86_64.
#
# With NATIVE_WEBVIEW_STAGE=<dir> the programs are only built: each lands in
# <dir>/<triple>-<backend> next to its mock DLL. Then `--run <dir>` runs them, on
# a Windows machine without wine or Xvfb; NATIVE_WEBVIEW_RUNNER prefixes a command
# such as `xvfb-run -a wine` to try the stage out elsewhere.
#
# Usage: tools/bash/native-windows-webview-check.sh <salam>
#        tools/bash/native-windows-webview-check.sh --run <dir>

set -u
pass=0
fail=0

check_output() {
    # check_output <name> <got> <expect>
    if [ "$(tr -d '\r' <"$2")" = "$(tr -d '\r' <"$3")" ]; then
        echo "ok   $1"
        pass=$((pass + 1))
    else
        echo "FAIL $1"
        tr -d '\r' <"$2" | head -20
        fail=$((fail + 1))
    fi
}

if [ "${1:-}" = "--run" ]; then
    DIR=$2
    for d in "$DIR"/*/; do
        d=${d%/}
        [ -f "$d/t.exe" ] || continue
        # shellcheck disable=SC2086
        (cd "$d" && timeout 120 ${NATIVE_WEBVIEW_RUNNER:-} ./t.exe </dev/null >got 2>err)
        rc=$?
        if [ "$rc" -ne 0 ]; then
            echo "exit $rc from $(basename "$d")"
            head -5 "$d/err"
        fi
        check_output "$(basename "$d")" "$d/got" "$d/expect"
    done
    echo "webview check: $pass passed, $fail failed"
    [ "$fail" -eq 0 ] && [ "$pass" -gt 0 ]
    exit
fi

SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
MOCK="$(cd "$(dirname "$0")/../.." && pwd)/std/gui/webview/native/mock/webview2_mock.c"
STAGE=${NATIVE_WEBVIEW_STAGE:-}
if [ -n "$STAGE" ]; then
    mkdir -p "$STAGE"
    STAGE=$(cd "$STAGE" && pwd)
fi
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
export WINEDEBUG=-all

cat >"$work/main.salam" <<'SALAM'
import gui.webview
import str

func main():
    webview.RegisterScheme("app", (r: webview.Request):
        println "scheme", r.scheme, r.method, r.path, r.query
        body := "hello"
        mut res := webview.Response { mime = "text/plain", data = body as void*, size = 5 as i64 }
        res.Header("X-Salam", "1")
        ret res
    end)
    mut w := webview.Create(false)
    println "closed", w.IsClosed()
    w.SetTitle("title")
    w.SetSize(640, 480)
    w.SetContextMenu(false)
    w.Init("window.a = 1")
    w.Init("window.b = 2", true)
    w.OnMessage((m: str): println "message", m end)
    w.OnNavigate((u: str):
        println "guard", u
        ret not str.Contains(u, "blocked")
    end)
    w.OnNewWindow((u: str): println "popup", u end)
    w.OnLoad((u: str, done: bool): println "load", u, done end)
    w.OnCrash((): println "crash" end)
    w.Navigate(webview.SchemeURL("app", "/index.html?x=1"))
    w.Navigate("https://blocked.example/")
    println "url", w.URL()
    w.SetHTML("<p>سلام</p>")
    repeat 1000:
        w.Eval("1 + 1")
    end
    w.Destroy()
    println "done", w.IsClosed()
end
SALAM

cat >"$work/expect" <<'OUT'
filter https://app.localhost/*
closed false
script if(window===window.top){window.a = 1}
script window.b = 2
navigate https://app.localhost/index.html?x=1
guard https://app.localhost/index.html?x=1
load https://app.localhost/index.html?x=1 false
scheme app GET /index.html x=1
response 200 body=hello
reason OK
headers Content-Type: text/plain
X-Salam: 1
message {"hello":1}
new window handled 1
popup https://popup.example/
crash
load https://app.localhost/index.html?x=1 true
navigate https://blocked.example/
guard https://blocked.example/
navigation cancelled
url https://app.localhost/index.html?x=1
html <p>سلام</p>
exec 1 + 1
close exec=1000 bad_stack=0 bounds=1
done true
OUT

for triple in ${NATIVE_WEBVIEW_TRIPLES:-i686-w64-windows-gnu x86_64-w64-windows-gnu}; do
    tool=${triple%%-*}-w64-mingw32
    d="$work/$triple"
    mkdir -p "$d"
    if ! "$tool-gcc" -shared "$MOCK" -o "$d/WebView2Loader.dll" -Wl,--kill-at -lole32 >"$d/mock.log" 2>&1; then
        echo "FAIL $triple mock does not build"
        head -5 "$d/mock.log"
        fail=$((fail + 1))
        continue
    fi
    for backend in native llvm; do
        name="$triple $backend"
        if ! (cd "$d" && "$SALAM" build "$work/main.salam" --backend="$backend" --target="$triple" --libpath="$d" \
            --output="$d/$backend.exe" --log-level=error >"$d/$backend.log" 2>&1); then
            echo "FAIL $name (build)"
            head -5 "$d/$backend.log"
            fail=$((fail + 1))
            continue
        fi
        if [ -n "$STAGE" ]; then
            s="$STAGE/$triple-$backend"
            mkdir -p "$s"
            cp "$d/$backend.exe" "$s/t.exe"
            cp "$d/WebView2Loader.dll" "$work/expect" "$s/"
            pass=$((pass + 1))
            continue
        fi
        (cd "$d" && timeout 120 xvfb-run -a wine "./$backend.exe" </dev/null >"$d/$backend.out" 2>/dev/null)
        check_output "$name" "$d/$backend.out" "$work/expect"
    done
done

if [ -n "$STAGE" ]; then
    echo "webview check: $pass staged in $STAGE, $fail failed to build"
else
    echo "webview check: $pass passed, $fail failed"
fi
[ "$fail" -eq 0 ]
