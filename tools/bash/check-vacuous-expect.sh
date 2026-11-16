#!/bin/sh
# usage: check-vacuous-expect.sh <salam-binary> [test dir ...]
# Fails when a page test has an EXPECT line that the empty page already contains.
SALAM=$(cd "$(dirname "$1")" && pwd)/$(basename "$1")
shift
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
[ $# -gt 0 ] || set -- "$ROOT"/tests/en/page/elements "$ROOT"/tests/en/page/style "$ROOT"/tests/en/page/components "$ROOT"/tests/en/page/pages "$ROOT"/tests/fa/page/elements "$ROOT"/tests/fa/page/style "$ROOT"/tests/fa/page/components "$ROOT"/tests/fa/page/pages
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
printf 'page:\nend\n' >"$WORK/empty.salam"
bad=0
total=0
for lang in en fa; do
    "$SALAM" page build "$WORK/empty.salam" --inline --no-minify --output="$WORK/empty_$lang.html" --no-color --log-level=error --lang="$lang" >/dev/null 2>&1 || {
        echo "cannot build the empty page ($lang)" >&2
        exit 2
    }
done
for dir in "$@"; do
    case "$dir" in */fa/*) lang=fa ;; *) lang=en ;; esac
    for f in "$dir"/*.salam; do
        [ -e "$f" ] || continue
        total=$((total + 1))
        grep -oE '(EXPECT|انتظار|توقع): .*' "$f" | sed -E 's/^(EXPECT|انتظار|توقع): //' | tr -d '\r' |
            while IFS= read -r line; do
                [ -n "$line" ] || continue
                if grep -qF -- "$line" "$WORK/empty_$lang.html"; then
                    echo "VACUOUS $f: $line"
                fi
            done
    done
done >"$WORK/out"
cat "$WORK/out"
[ -s "$WORK/out" ] && bad=1
echo "checked $total tests, $(wc -l <"$WORK/out" | tr -d ' ') vacuous EXPECT lines"
exit $bad
