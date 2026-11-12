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

# CI splits the examples and the lesson snippets over several runners and gives
# each one a slice with WEBSITE_EXAMPLE_SHARD=N/M. The slices are round-robin,
# so a shard gets the same mix of heavy and light files as the others.
shard=${WEBSITE_EXAMPLE_SHARD:-}
shard_index=1
shard_total=1
if [ -n "$shard" ]; then
    case "$shard" in
    */*/* | /* | */ | *[!0-9/]*)
        echo "website: WEBSITE_EXAMPLE_SHARD must be N/M, not '$shard'" >&2
        exit 2
        ;;
    esac
    shard_index=${shard%/*}
    shard_total=${shard#*/}
    if [ "$shard_index" -lt 1 ] || [ "$shard_total" -lt 1 ] || [ "$shard_index" -gt "$shard_total" ]; then
        echo "website: WEBSITE_EXAMPLE_SHARD needs 1 <= N <= M, not '$shard'" >&2
        exit 2
    fi
fi

shard_list() {
    if [ "$shard_total" -gt 1 ]; then
        awk -v part="$shard_index" -v total="$shard_total" 'NR % total == (part - 1) % total' "$1"
    else
        cat "$1"
    fi
}

"$SALAM" run tools/salam/check-fa-names.salam --no-color --log-level=error
"$SALAM" run website/check-std-coverage.salam --no-color --log-level=error
"$SALAM" run website/check-code-words.salam --no-color --log-level=error

jobs=$(nproc 2>/dev/null || echo 4)
if [ "$examples" != skip ]; then
    fail=0
    list=$(mktemp)
    printf '%s\n' website/content/*/examples/*.salam website/content/*/std-examples/*/*.salam >"$list"
    shard_list "$list" >"$list.shard"
    echo "website: checking $(wc -l <"$list.shard" | tr -d ' ') examples (shard $shard_index/$shard_total, $jobs at a time)"
    tr '\n' '\0' <"$list.shard" |
        xargs -0 -P "$jobs" -n 1 sh website/check-example.sh || fail=1
    rm -f "$list" "$list.shard"
    [ "$fail" -eq 0 ] || {
        echo "website: example output does not match its .out file" >&2
        exit 1
    }
    lessons=$(mktemp)
    printf '%s\n' website/content/*/learn/*.txt >"$lessons"
    shard_list "$lessons" >"$lessons.shard"
    if [ -s "$lessons.shard" ]; then
        echo "website: checking the snippets of $(wc -l <"$lessons.shard" | tr -d ' ') lessons (shard $shard_index/$shard_total)"
        # shellcheck disable=SC2046 # lesson paths never contain spaces
        CHECK_JOBS="$jobs" "$SALAM" run tools/salam/check-snippets.salam $(cat "$lessons.shard") --no-color --log-level=error || {
            echo "website: a lesson snippet does not compile" >&2
            rm -f "$lessons" "$lessons.shard"
            exit 1
        }
    fi
    rm -f "$lessons" "$lessons.shard"
    if [ "$shard_index" -eq 1 ]; then
        sh website/check-blog-code.sh || {
            echo "website: a magazine code block does not compile or its output differs" >&2
            exit 1
        }
    fi
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

"$SALAM" run tools/seo/check-site.salam website/dist/fa website/dist/en --no-color --log-level=error
