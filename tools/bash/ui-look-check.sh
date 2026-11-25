#!/bin/sh
# Same-look check: renders each page of a directory twice, once as a web page
# in headless Chromium and once through the native renderer (the program that
# --ui=native generates, drawn by ui.RenderPage into a PNG), then compares the two PNGs pixel by pixel.
#
# Usage:
#   tools/bash/ui-look-check.sh --salam PROG [--pages DIR] [--out DIR]
#                               [--tolerance PERCENT] [--fuzz PERCENT]
#                               [--size WxH] [--chrome PROG]
#
# A page passes when at most --tolerance percent of its pixels differ by more
# than --fuzz percent per channel. The result table goes to stdout and to
# OUT/report.txt; the screenshots and the diff images stay in OUT.
#
# Env:
#   LOOK_STRICT=1   exit 1 when any page is over the tolerance (default: 0,
#                   the check reports and never fails the build)
#   SALAM_STD       std directory handed to the compiler

set -u

SALAM=
PAGES=
OUT=
TOLERANCE=2
FUZZ=12
SIZE=800x600
CHROME=${LOOK_CHROME:-}

while [ $# -gt 0 ]; do
    case $1 in
    --salam)
        SALAM=$2
        shift 2
        ;;
    --pages)
        PAGES=$2
        shift 2
        ;;
    --out)
        OUT=$2
        shift 2
        ;;
    --tolerance)
        TOLERANCE=$2
        shift 2
        ;;
    --fuzz)
        FUZZ=$2
        shift 2
        ;;
    --size)
        SIZE=$2
        shift 2
        ;;
    --chrome)
        CHROME=$2
        shift 2
        ;;
    -h | --help)
        sed -n '2,19p' "$0"
        exit 0
        ;;
    *)
        echo "unknown argument: $1" >&2
        exit 2
        ;;
    esac
done

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
[ -n "$PAGES" ] || PAGES=$ROOT/tools/ui-look/pages
[ -n "$OUT" ] || OUT=$(pwd)/ui-look-out
[ -n "$SALAM" ] || SALAM=$(command -v salam 2>/dev/null || true)
[ -n "$SALAM" ] || {
    echo "error: --salam PROG is required" >&2
    exit 2
}
case "$SALAM" in /*) ;; *) SALAM=$(pwd)/$SALAM ;; esac
case "$OUT" in /*) ;; *) OUT=$(pwd)/$OUT ;; esac
case "$PAGES" in /*) ;; *) PAGES=$(pwd)/$PAGES ;; esac

if [ -z "$CHROME" ]; then
    for c in google-chrome chromium chromium-browser chrome; do
        if command -v "$c" >/dev/null 2>&1; then
            CHROME=$c
            break
        fi
    done
fi
[ -n "$CHROME" ] || {
    echo "error: no Chromium or Chrome found (set LOOK_CHROME or --chrome)" >&2
    exit 2
}

if command -v magick >/dev/null 2>&1; then
    IM_COMPARE="magick compare"
    IM_CONVERT="magick"
else
    IM_COMPARE="compare"
    IM_CONVERT="convert"
fi

W=${SIZE%x*}
H=${SIZE#*x}

[ -z "${SALAM_STD:-}" ] && [ -d "$ROOT/std" ] && SALAM_STD=$ROOT/std
export SALAM_STD

rm -rf "$OUT"
mkdir -p "$OUT"
REPORT=$OUT/report.txt
: >"$REPORT"
over=0
total=0

say() {
    printf '%s\n' "$1" | tee -a "$REPORT"
}

say "same-look check: ${W}x${H}, tolerance ${TOLERANCE}%, fuzz ${FUZZ}%"

for f in "$PAGES"/*.salam; do
    [ -e "$f" ] || continue
    name=$(basename "$f" .salam)
    work=$OUT/$name
    mkdir -p "$work"
    total=$((total + 1))

    if ! (cd "$work" && "$SALAM" build "$f" --platform=web --inline --no-minify --output="$work/web.html" --no-color --log-level=error) >"$work/web-build.log" 2>&1; then
        say "ERROR $name (web build failed: $(head -1 "$work/web-build.log"))"
        over=$((over + 1))
        continue
    fi
    (cd "$work" && timeout 60 "$CHROME" --headless=new --no-sandbox --disable-gpu --hide-scrollbars \
        --force-device-scale-factor=1 --window-size="$W,$H" --virtual-time-budget=2000 \
        --screenshot="$work/web.png" "file://$work/web.html") >"$work/chrome.log" 2>&1
    if [ ! -s "$work/web.png" ]; then
        say "ERROR $name (no web screenshot: $(tail -1 "$work/chrome.log"))"
        over=$((over + 1))
        continue
    fi

    (cd "$work" && SALAM_NATIVE_DUMP=1 "$SALAM" build "$f" --ui=native --output="$work/dump-only" --no-color --log-level=error) >"$work/native-dump.log" 2>&1
    awk '
        /^func main:/ { skipping = 1; next }
        skipping && /^end/ { skipping = 0; next }
        skipping { next }
        /^(error|\[[0-9-]+ [0-9:]+\])/ { exit }
        { print }
    ' "$work/native-dump.log" >"$work/native.salam"
    if ! grep -q '__salam_page' "$work/native.salam" || grep -q 'build aborted' "$work/native-dump.log"; then
        say "ERROR $name (no native program: $(grep -m1 "^error" "$work/native-dump.log" || head -1 "$work/native-dump.log"))"
        over=$((over + 1))
        continue
    fi
    {
        echo 'import gui.paint'
        echo 'import os'
        sed -E 's/^( *__salam_touched_[0-9]+) \|= 0$/\1 = 0/' "$work/native.salam"
        echo 'func main:'
        echo "    cv := ui.RenderPage((): ret ui.NoPatches() end, (__salam_b: ui.Builder): __salam_page(__salam_b) end, $W, $H)"
        echo '    if not paint.SavePNG(cv, os.Env("LOOK_SHOT")):'
        echo '        println "cannot write the screenshot"'
        echo '    end'
        echo 'end'
    } >"$work/native-shot.salam"
    if ! (cd "$work" && "$SALAM" build "$work/native-shot.salam" --output="$work/app" --no-color --log-level=error) >"$work/native-build.log" 2>&1; then
        say "ERROR $name (native build failed: $(head -1 "$work/native-build.log"))"
        over=$((over + 1))
        continue
    fi
    (cd "$work" && LOOK_SHOT="$work/native.png" SALAM_WINDOW_BACKEND=headless timeout 60 ./app) >"$work/native-run.log" 2>&1
    if [ ! -s "$work/native.png" ]; then
        say "ERROR $name (no native screenshot: $(tail -1 "$work/native-run.log"))"
        over=$((over + 1))
        continue
    fi

    $IM_CONVERT "$work/web.png" -background white -alpha remove -gravity northwest -extent "${W}x${H}" "$work/web-fit.png"
    $IM_CONVERT "$work/native.png" -background white -alpha remove -gravity northwest -extent "${W}x${H}" "$work/native-fit.png"
    cmpout=$($IM_COMPARE -metric AE -fuzz "${FUZZ}%" "$work/web-fit.png" "$work/native-fit.png" "$OUT/$name-diff.png" 2>&1)
    cmprc=$?
    diffpx=$(printf '%s' "$cmpout" | awk '{ print $1 }')
    case "$diffpx" in
    '' | *[!0-9]*)
        say "ERROR $name (compare failed: $cmpout)"
        over=$((over + 1))
        continue
        ;;
    esac
    if [ "$cmprc" -gt 1 ]; then
        say "ERROR $name (compare exited $cmprc: $cmpout)"
        over=$((over + 1))
        continue
    fi
    permille=$((diffpx * 1000 / (W * H)))
    pct="$((permille / 10)).$((permille % 10))"
    limit=$((TOLERANCE * 10))
    if [ "$permille" -le "$limit" ]; then
        say "PASS $name (${pct}% differ)"
    else
        say "DIFF $name (${pct}% differ, over ${TOLERANCE}%)"
        over=$((over + 1))
    fi
done

say "checked $total pages, $over over the tolerance or broken"
[ "${LOOK_STRICT:-0}" = 1 ] && [ "$over" -gt 0 ] && exit 1
exit 0
