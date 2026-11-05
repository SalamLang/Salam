#!/bin/sh
# Bundles tests/en/desktop/services for Windows and runs it under wine next to the
# WebView2 mock, which exercises real wine implementations of the Credential
# Manager, processes, menus, tray and the asset pack inside the executable.
#
# Needs wine, Xvfb and a mingw-w64 gcc for each target.
#
# Usage: tools/bash/desktop-wine-check.sh <salam>

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
root=$(cd "$(dirname "$0")/../.." && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
export WINEDEBUG=-all
fail=0
for arch in ${DESKTOP_WINE_ARCHES:-x86_64 i686}; do
    d="$work/$arch"
    if ! "$SALAM" bundle "$root/tests/en/desktop/services" --target="$arch-w64-windows-gnu" --output="$d" >"$d.log" 2>&1; then
        echo "FAIL $arch (bundle)"
        tail -5 "$d.log"
        fail=1
        continue
    fi
    (cd "$d" && python3 -I -c "import zipfile,glob;zipfile.ZipFile(glob.glob('*.zip')[0]).extractall('.')")
    app=$(dirname "$(find "$d" -name '*.exe' | head -1)")
    "$arch-w64-mingw32-gcc" -shared "$root/std/gui/webview/native/mock/webview2_mock.c" -o "$app/WebView2Loader.dll" -Wl,--kill-at -lole32
    (cd "$app" && timeout 120 xvfb-run -a wine ./salam-services.exe </dev/null >got 2>/dev/null)
    rc=$?
    tr -d '\r' <"$app/got" | grep -E '^(FAIL|done)' | sed "s/^/$arch: /"
    if [ "$rc" -ne 0 ] || grep -q '^FAIL' "$app/got" || ! grep -q '^done true' "$app/got" || ! grep -q 'packed page' "$app/got"; then
        echo "FAIL $arch (exit $rc)"
        fail=1
    else
        echo "ok   $arch"
    fi
done
exit "$fail"
