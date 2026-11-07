#!/bin/sh

set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
. "$root/tools/bash/lib.sh"
cd "$root"
salam_ensure_compiler
case "$SALAM" in
/*) ;;
*/*) SALAM="$root/$SALAM" ;;
esac
export SALAM

examples=${WEBSITE_EXAMPLES:-check}
case "$examples" in
check | skip | only) ;;
*)
    echo "website: WEBSITE_EXAMPLES must be check, skip or only, not '$examples'" >&2
    exit 2
    ;;
esac

"$SALAM" run tools/salam/check-fa-names.salam --no-color --log-level=error
"$SALAM" run website/check-std-coverage.salam --no-color --log-level=error
"$SALAM" run website/check-code-words.salam --no-color --log-level=error

jobs=$(nproc 2>/dev/null || echo 4)
if [ "$examples" != skip ]; then
    fail=0
    printf '%s\0' website/content/*/examples/*.salam website/content/*/std-examples/*/*.salam |
        xargs -0 -P "$jobs" -n 1 sh website/check-example.sh || fail=1
    [ "$fail" -eq 0 ] || {
        echo "website: example output does not match its .out file" >&2
        exit 1
    }
    "$SALAM" run tools/salam/check-snippets.salam --no-color --log-level=error || {
        echo "website: a lesson snippet does not compile" >&2
        exit 1
    }
fi
[ "$examples" = only ] && exit 0

if command -v sha256sum >/dev/null 2>&1; then
    sha=sha256sum
else
    sha="shasum -a 256"
fi

cache="$root/website/.cache"
mkdir -p "$cache/api"
compiler_stamp=$($sha "$SALAM" | cut -d ' ' -f 1)
SITE_PREVIEW_CACHE="$cache/preview"
export SITE_PREVIEW_CACHE

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
mkdir -p "$work/gen"
(cd "$work/gen" && "$SALAM" build "$root/website/build.salam" --output="$work/generator" --no-color --log-level=error >"$work/gen.log" 2>&1) &
gen_pid=$!

mkdir -p website/dist/api
: >"$work/docs"
: >"$work/seen"
for page in website/content/*/std/*.txt; do
    [ -f "$page" ] || continue
    pkg=$(sed -n 's/^pkg\.en = //p' "$page")
    slug=$(printf '%s' "$pkg" | tr '/' '-')
    out="$cache/api/$slug.json"
    grep -qxF "$out" "$work/seen" && continue
    printf '%s\n' "$out" >>"$work/seen"
    pkg_stamp=$({
        printf '%s\n' "$compiler_stamp"
        find "std/$pkg" -type f -exec $sha {} + | LC_ALL=C sort
    } | $sha | cut -d ' ' -f 1)
    if [ -s "$out" ] && [ "$(cat "$out.stamp" 2>/dev/null)" = "$pkg_stamp" ]; then
        continue
    fi
    src="std/$pkg"
    if grep -q '^pkg\.flat = true' "$page"; then
        src=$(mktemp -d "$work/flat.XXXXXX")
        cp "std/$pkg"/*.salam "$src/"
    fi
    printf '%s\n' "$pkg_stamp" >"$out.stamp.want"
    printf '%s\n%s\n%s\n' "$src" "$out" "$pkg" >>"$work/docs"
done
if [ -s "$work/docs" ] &&
    ! tr '\n' '\0' <"$work/docs" |
    xargs -0 -n 3 -P "$jobs" sh -c '"$SALAM" doc "$1" --output="$2" >/dev/null || { echo "website: salam doc failed for std/$3" >&2; exit 1; }' sh; then
    kill "$gen_pid" 2>/dev/null || true
    exit 1
fi
for want_stamp in "$cache/api"/*.json.stamp.want; do
    [ -f "$want_stamp" ] || continue
    [ -s "${want_stamp%.stamp.want}" ] || continue
    mv "$want_stamp" "${want_stamp%.want}"
done
cp "$cache/api"/*.json website/dist/api/

if ! wait "$gen_pid"; then
    cat "$work/gen.log" >&2
    exit 1
fi
"$work/generator" "$@"

for want in \
    'en/std/dependency/index.html|<span class="tok-k">import</span> dependency' \
    'en/std/dependency/index.html|<span class="tok-t">str</span>' \
    'fa/std/dependency/index.html|<span class="tok-k">واردسازی</span> وابستگی' \
    'fa/std/dependency/index.html|<span class="tok-b">سرچاپ</span>' \
    'en/std/dependency/index.html|<span class="tok-k">interface</span> <span class="tok-t">Greeter</span>:' \
    'en/std/dependency/index.html|<span class="tok-k">func</span> <span class="tok-f">main</span>:' \
    'fa/std/dependency/index.html|<span class="tok-k">تابع</span> <span class="tok-f">ریشه</span>:'; do
    page=${want%%|*}
    text=${want#*|}
    if [ -f "website/dist/$page" ] && ! grep -qF "$text" "website/dist/$page"; then
        echo "website: $page lacks the highlighted code '$text'" >&2
        exit 1
    fi
done
