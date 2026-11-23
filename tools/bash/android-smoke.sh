#!/bin/sh
# Installs APK, drives it with a salam-script and checks that the smoke page
# wrote files/tapped.txt.
#
# Usage: tools/bash/android-smoke.sh APK PACKAGE OUTDIR
set -u
APK=$1
PKG=$2
OUT=$3
ADB=${ADB:-adb}
mkdir -p "$OUT"
"$ADB" install -r "$APK" || exit 1
"$ADB" logcat -c
"$ADB" shell "am start -n $PKG/ir.salamlang.host.SalamActivity --es salam-script 'wait 3000;tap 100,200;wait 1000;tap 100,200'" || exit 1
i=0
ok=0
while [ $i -lt 45 ]; do
    if "$ADB" shell "run-as $PKG cat files/tapped.txt" 2>/dev/null | grep -q tapped; then
        ok=1
        break
    fi
    sleep 2
    i=$((i + 1))
done
sleep 1
"$ADB" exec-out screencap -p >"$OUT/smoke.png" || true
"$ADB" logcat -d -s salam >"$OUT/logcat.txt" || true
if [ $ok -ne 1 ]; then
    tail -40 "$OUT/logcat.txt"
    echo "android-smoke: the page did not record a tap" >&2
    exit 1
fi
echo "android-smoke: tap recorded"
