#!/bin/sh
# Downloads a released salam for a platform other than this machine's, for
# builds that happen inside a foreign-architecture container. setup-salam
# installs the *runner's* architecture, which is the wrong one when the compile
# runs under docker --platform=linux/386 or arm32v7.
#
# Usage:
#   tools/bash/fetch-seed.sh --slug linux-i686 --out DIR [--version X.Y.Z]
#
# Without --version the newest published release is used.

set -eu

SLUG=
OUT=$(pwd)/seed
VERSION=
REPO=${SALAM_REPO:-SalamLang/Salam}

while [ $# -gt 0 ]; do
    case $1 in
    --slug)
        SLUG=$2
        shift 2
        ;;
    --slug=*)
        SLUG=${1#*=}
        shift
        ;;
    --out)
        OUT=$2
        shift 2
        ;;
    --out=*)
        OUT=${1#*=}
        shift
        ;;
    --version)
        VERSION=$2
        shift 2
        ;;
    --version=*)
        VERSION=${1#*=}
        shift
        ;;
    -h | --help)
        sed -n '2,10p' "$0"
        exit 0
        ;;
    *)
        echo "unknown argument: $1" >&2
        exit 2
        ;;
    esac
done

[ -n "$SLUG" ] || {
    echo "error: --slug is required (linux-i686, linux-armhf, ...)" >&2
    exit 2
}

resolve_tags() {
    api="https://api.github.com/repos/$REPO/releases?per_page=50"
    tok=${GH_TOKEN:-${GITHUB_TOKEN:-}}
    if [ -n "$tok" ]; then
        body=$(curl -fsSL --retry 3 -H "Authorization: Bearer $tok" -H "Accept: application/vnd.github+json" "$api") || body=
    else
        body=$(curl -fsSL --retry 3 -H "Accept: application/vnd.github+json" "$api") || body=
    fi
    printf '%s' "${body:-}" \
        | sed -nE 's/.*"tag_name"[[:space:]]*:[[:space:]]*"([^"]+)".*/\1/p' \
        | grep -E '^v[0-9]+\.[0-9]+\.[0-9]+$' || true
}

tags=
if [ -z "$VERSION" ]; then
    tags=$(resolve_tags)
    [ -n "$tags" ] || {
        echo "error: could not read compiler release tags from github.com or the API" >&2
        echo "       (the API allows 60 unauthenticated calls an hour per IP - set" >&2
        echo "        GH_TOKEN, or pass --version to skip the lookup entirely)" >&2
        exit 1
    }
else
    case "$VERSION" in v*) preferred="$VERSION" ;; *) preferred="v$VERSION" ;; esac
    tags=$(printf '%s\n%s\n' "$preferred" "$(resolve_tags)" | awk 'NF && !seen[$0]++')
fi

rm -rf "$OUT"
mkdir -p "$OUT"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT INT TERM

asset=
tag=
url=
tried=
for t in $tags; do
    v=${t#v}
    candidate="salam-${v}-${SLUG}.zip"
    candidate_url="https://github.com/$REPO/releases/download/$t/$candidate"
    tried="${tried} ${candidate}"
    if curl -fsIL --retry 2 -o /dev/null "$candidate_url"; then
        tag=$t
        VERSION=$v
        asset=$candidate
        url=$candidate_url
        break
    fi
done

[ -n "$asset" ] || {
    echo "error: no published seed asset found for slug '$SLUG'; tried:${tried}" >&2
    exit 1
}

echo "seed asset : $asset ($tag)"
curl -fsSL --retry 3 -o "$tmp/seed.zip" "$url"
unzip -q "$tmp/seed.zip" -d "$tmp/x"

bin=$(find "$tmp/x" -name salam -type f 2>/dev/null | sed -n '1p')
[ -n "$bin" ] || bin=$(find "$tmp/x" -name 'salam.exe' -type f 2>/dev/null | sed -n '1p')
[ -n "$bin" ] || {
    echo "error: no salam binary inside $asset" >&2
    exit 1
}
cp -r "$(dirname "$bin")"/. "$OUT/"
chmod +x "$OUT/$(basename "$bin")" 2>/dev/null || true
echo "seed       : $OUT/$(basename "$bin")"
