#!/bin/sh

set -eu

REPO="SalamLang/Salam"
INSTALL_DIR="${SALAM_INSTALL_DIR:-/opt/salam}"
VERSION="${SALAM_VERSION:-}"

log() { printf '%s\n' "$*" >&2; }
die() {
    log "error: $*"
    exit 1
}

while [ $# -gt 0 ]; do
    case "$1" in
    --dir | -d)
        [ $# -ge 2 ] || die "$1 requires a value"
        INSTALL_DIR="$2"
        shift 2
        ;;
    --dir=*)
        INSTALL_DIR="${1#--dir=}"
        shift
        ;;
    --version | -v)
        [ $# -ge 2 ] || die "$1 requires a value"
        VERSION="$2"
        shift 2
        ;;
    --version=*)
        VERSION="${1#--version=}"
        shift
        ;;
    -h | --help)
        sed -n '2,16p' "$0"
        exit 0
        ;;
    *) die "unknown option: $1" ;;
    esac
done

API_TOKEN="${GITHUB_TOKEN:-${GH_TOKEN:-}}"

fetch_to_stdout() {
    auth=""
    case "$1" in
    https://api.github.com/*)
        if [ -n "$API_TOKEN" ]; then
            auth="Authorization: Bearer $API_TOKEN"
        fi
        ;;
    esac
    if command -v curl >/dev/null 2>&1; then
        if [ -n "$auth" ]; then
            curl -fsSL --retry 3 --retry-delay 2 -H "$auth" "$1"
        else
            curl -fsSL --retry 3 --retry-delay 2 "$1"
        fi
    elif command -v wget >/dev/null 2>&1; then
        if [ -n "$auth" ]; then
            wget -qO- --tries=3 --header="$auth" "$1"
        else
            wget -qO- --tries=3 "$1"
        fi
    else
        die "need curl or wget to install Salam"
    fi
}

list_release_tags() {
    json="$(fetch_to_stdout "https://api.github.com/repos/${REPO}/releases?per_page=10" 2>/dev/null || true)"
    found_tags="$(printf '%s' "$json" | sed -n 's/.*"tag_name": *"\([^"]*\)".*/\1/p')"
    if [ -z "$found_tags" ]; then
        log "  releases API unavailable, falling back to the Atom feed"
        atom="$(fetch_to_stdout "https://github.com/${REPO}/releases.atom" 2>/dev/null || true)"
        found_tags="$(printf '%s' "$atom" | sed -n 's#.*/releases/tag/\([^"]*\)".*#\1#p')"
    fi
    printf '%s\n' "$found_tags" | grep -v nightly || true
}

fetch_to_file() {
    if command -v curl >/dev/null 2>&1; then
        curl -fL --retry 3 --retry-delay 2 -o "$2" "$1"
    elif command -v wget >/dev/null 2>&1; then
        wget -q --tries=3 -O "$2" "$1"
    else
        die "need curl or wget to install Salam"
    fi
}

os="$(uname -s)"
[ "$os" = "Linux" ] || die "this installer is for the Salam Docker image (Linux only), got: $os"

arch="$(uname -m)"
case "$arch" in
x86_64 | amd64)
    platforms="linux-x86_64 linux"
    ;;
i386 | i486 | i586 | i686 | x86)
    platforms="linux-i686"
    ;;
aarch64 | arm64)
    platforms="linux-aarch64"
    ;;
armv6l | armv7l | armv7 | arm)
    platforms="linux-armhf"
    ;;
*)
    die "unsupported architecture: $arch"
    ;;
esac
platform="${platforms%% *}"

workdir="$(mktemp -d 2>/dev/null || mktemp -d -t salam)"
trap 'rm -rf "$workdir"' EXIT INT TERM
archive="$workdir/download.zip"

if [ -n "$VERSION" ]; then
    found=0
    for p in $platforms; do
        asset="salam-${VERSION}-${p}.zip"
        url="https://github.com/${REPO}/releases/download/v${VERSION}/${asset}"
        if fetch_to_file "$url" "$archive" 2>/dev/null && [ -s "$archive" ]; then
            platform="$p"
            ASSET="$asset"
            URL="$url"
            found=1
            break
        fi
        rm -f "$archive"
    done
    [ "$found" = 1 ] || die "no asset for $arch in release v${VERSION}"
    log "Installing Salam ${VERSION} (${platform}) from:"
    log "  $URL"
else
    log "Resolving latest Salam release with a ${platform} asset..."
    tags="$(list_release_tags)"
    lookup_err="could not list releases for ${REPO}: both"
    lookup_err="$lookup_err https://api.github.com/repos/${REPO}/releases"
    lookup_err="$lookup_err and https://github.com/${REPO}/releases.atom failed."
    lookup_err="$lookup_err Pass --version to skip the lookup."
    [ -n "$tags" ] || die "$lookup_err"

    found=0
    for tag in $tags; do
        v="${tag#v}"
        log "  trying ${tag}..."
        for p in $platforms; do
            asset="salam-${v}-${p}.zip"
            url="https://github.com/${REPO}/releases/download/${tag}/${asset}"
            if fetch_to_file "$url" "$archive" 2>/dev/null && [ -s "$archive" ]; then
                platform="$p"
                VERSION="$v"
                ASSET="$asset"
                URL="$url"
                found=1
                break
            fi
            rm -f "$archive"
        done
        if [ "$found" = 1 ]; then
            break
        fi
    done
    [ "$found" = 1 ] || die "no release under https://github.com/${REPO}/releases publishes a ${platform} asset"
    log "Installing Salam ${VERSION} (${platform}) from:"
    log "  $URL"
fi

extract_dir="$workdir/extracted"
mkdir -p "$extract_dir"
if command -v unzip >/dev/null 2>&1; then
    unzip -q "$archive" -d "$extract_dir"
elif command -v bsdtar >/dev/null 2>&1; then
    bsdtar -xf "$archive" -C "$extract_dir"
else
    die "need unzip or bsdtar to extract the release archive"
fi

bundle_dir="$extract_dir/salam-${platform}"
[ -d "$bundle_dir" ] || bundle_dir="$(find "$extract_dir" -maxdepth 1 -mindepth 1 -type d | head -n 1)"
[ -d "$bundle_dir" ] || die "could not find the salam bundle inside $ASSET"
[ -f "$bundle_dir/salam" ] || die "could not find 'salam' binary inside $ASSET"
[ -d "$bundle_dir/std" ] || die "could not find 'std/' inside $ASSET"

mkdir -p "$INSTALL_DIR"
cp -r "$bundle_dir"/. "$INSTALL_DIR"/
chmod +x "$INSTALL_DIR/salam"
printf '%s' "$VERSION" >"$INSTALL_DIR/VERSION"

log "Installed Salam ${VERSION} to $INSTALL_DIR"
"$INSTALL_DIR/salam" version >&2 2>/dev/null || true
