#!/bin/sh
# Builds a std/webview program for Windows against a mock WebView2Loader.dll
# (std/webview/native/mock/webview2_mock.c) and runs it under wine, which checks
# the COM calling convention both ways: the program's vtable calls, and the
# handlers the mock calls back, which must pop their own arguments on i686.
#
# Needs wine on PATH, Xvfb (wine has no window driver without a display) and
# i686 and x86_64 mingw-w64 compilers.
#
# Usage: tools/bash/native-windows-webview-check.sh <salam>

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
MOCK="$(cd "$(dirname "$0")/../.." && pwd)/std/webview/native/mock/webview2_mock.c"
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
export WINEDEBUG=-all
pass=0
fail=0

cat >"$work/main.salam" <<'SALAM'
import webview

func main():
    mut w := webview.Create(false)
    println "closed", w.IsClosed()
    w.SetTitle("title")
    w.SetSize(640, 480)
    w.Navigate("https://example.com/")
    w.SetHTML("<p>سلام</p>")
    repeat 1000:
        w.Eval("1 + 1")
    end
    w.Destroy()
    println "done", w.IsClosed()
end
SALAM

cat >"$work/expect" <<'OUT'
bounds 0 0 800 600
closed false
bounds 0 0 640 480
navigate https://example.com/
html <p>سلام</p>
exec 1 + 1
close exec=1000 bad_stack=0
done true
OUT

for triple in i686-w64-windows-gnu x86_64-w64-windows-gnu; do
    case "$triple" in
        i686-*)
            tool=i686-w64-mingw32
            decorated=@16
            ;;
        *)
            tool=x86_64-w64-mingw32
            decorated=
            ;;
    esac
    d="$work/$triple"
    mkdir -p "$d"
    printf 'LIBRARY WebView2Loader.dll\nEXPORTS\nCreateCoreWebView2EnvironmentWithOptions%s\n' "$decorated" >"$d/mock.def"
    if ! "$tool-gcc" -shared "$MOCK" -o "$d/WebView2Loader.dll" -Wl,--kill-at >"$d/mock.log" 2>&1 ||
        ! "$tool-dlltool" -d "$d/mock.def" -k -l "$d/libWebView2Loader.dll.a" >>"$d/mock.log" 2>&1; then
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
        (cd "$d" && timeout 120 xvfb-run -a wine "./$backend.exe" </dev/null >"$d/$backend.out" 2>/dev/null)
        if [ "$(tr -d '\r' <"$d/$backend.out")" = "$(cat "$work/expect")" ]; then
            echo "ok   $name"
            pass=$((pass + 1))
        else
            echo "FAIL $name"
            tr -d '\r' <"$d/$backend.out" | head -20
            fail=$((fail + 1))
        fi
    done
done

echo "webview check: $pass passed, $fail failed"
[ "$fail" -eq 0 ]
