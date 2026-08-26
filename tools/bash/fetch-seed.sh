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
        sed -n '2,12p' "$0"
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

# The newest release tag. github.com/OWNER/REPO/releases/latest redirects to
# .../releases/tag/vX.Y.Z, which is a plain web request; api.github.com answers
# the same question but allows only 60 unauthenticated calls an hour per IP,
# shared with everything else on that address. That is what a runner runs out
# of: the armhf job here got a 403 from the API while the i686 job beside it,
# on a different address, was fine. So ask github.com first and keep the API as
# the fallback, authenticated when a token is in the environment (1000 an hour
# per repository).
#
# Two seds rather than a pipeline into head, so a closed pipe cannot take the
# whole script down with SIGPIPE.
resolve_latest_tag() {
    eff=$(curl -fsSL --retry 3 -o /dev/null -w '%{url_effective}' \
        "https://github.com/$REPO/releases/latest" 2>/dev/null) || eff=
    case ${eff:-} in
    */releases/tag/*)
        printf '%s' "${eff##*/tag/}"
        return 0
        ;;
    esac
    api=https://api.github.com/repos/$REPO/releases/latest
    tok=${GH_TOKEN:-${GITHUB_TOKEN:-}}
    if [ -n "$tok" ]; then
        body=$(curl -fsSL --retry 3 -H "Authorization: Bearer $tok" "$api") || body=
    else
        body=$(curl -fsSL --retry 3 "$api") || body=
    fi
    printf '%s' "${body:-}" | sed -n 's/.*"tag_name"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' | sed -n '1p'
}

if [ -z "$VERSION" ]; then
    tag=$(resolve_latest_tag)
    [ -n "$tag" ] || {
        echo "error: could not read the latest release tag from github.com or the API" >&2
        echo "       (the API allows 60 unauthenticated calls an hour per IP - set" >&2
        echo "        GH_TOKEN, or pass --version to skip the lookup entirely)" >&2
        exit 1
    }
    VERSION=${tag#v}
else
    tag=v$VERSION
fi

asset="salam-${VERSION}-${SLUG}.zip"
url="https://github.com/$REPO/releases/download/$tag/$asset"

echo "seed asset : $asset ($tag)"
rm -rf "$OUT"
mkdir -p "$OUT"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT INT TERM
curl -fsSL --retry 3 -o "$tmp/seed.zip" "$url"
unzip -q "$tmp/seed.zip" -d "$tmp/x"

bin=$(find "$tmp/x" -name salam -type f 2>/dev/null | sed -n '1p')
[ -n "$bin" ] || bin=$(find "$tmp/x" -name 'salam.exe' -type f 2>/dev/null | sed -n '1p')
[ -n "$bin" ] || {
    echo "error: no salam binary inside $asset" >&2
    exit 1
}
# The whole tree, not just the binary: a release carries its own std/ beside it
# and some builds resolve through that.
cp -r "$(dirname "$bin")"/. "$OUT/"
chmod +x "$OUT/$(basename "$bin")" 2>/dev/null || true
echo "seed       : $OUT/$(basename "$bin")"
