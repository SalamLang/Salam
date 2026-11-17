#!/bin/sh
set -eu
SALAM=${1:?path to the salam compiler}
OUT=${2:-editor/index.html}
cd "$(dirname "$0")/../.."
"$SALAM" web editor/page.salam --output="$OUT" >/dev/null 2>&1
fail=0
need() {
    if ! grep -q -- "$1" "$OUT"; then
        echo "missing in $OUT: $1"
        fail=1
    fi
}
for id in menu-toggle controls mode-seg lang-seg examples dd-button dd-list theme autorun run \
    main-split edwrap ln hl code ac gutter view-seg status copy-code copy-out \
    anim-bar anim-prev anim-play anim-next anim-range anim-speed anim-loop anim-raw output preview scrim; do
    need "id=\"$id\""
done
for v in app layout en fa run html css js; do
    need "data-value=\"$v\""
done
need '<script>'
need 'color-scheme'
exit "$fail"
