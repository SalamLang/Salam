#!/bin/sh
# Installs a full Salam release bundle (the `salam` binary plus its std/
# library, LICENSE and README) for use inside the Salam Docker image.
#
# This differs from the repo-root install.sh (which installs only the
# `salam` binary onto a host PATH): programs that `use` std/ packages need
# std/ to sit next to the binary, so the whole release bundle is unpacked
# here, matching the layout the release zip already ships (salam + std/).
#
# Usage:
#   sh install.sh --dir /opt/salam
#   sh install.sh --dir /opt/salam --version 0.2.8
#
# Env vars (mirroring the root install.sh):
#   SALAM_INSTALL_DIR, SALAM_VERSION
#

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

fetch_to_stdout() {
    if command -v curl >/dev/null 2>&1; then
        curl -fsSL "$1"
    elif command -v wget >/dev/null 2>&1; then
        wget -qO- "$1"
    else
        die "need curl or wget to install Salam"
    fi
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
    platform="linux"
    ;;
i386 | i486 | i586 | i686 | x86)
    platform="linux-i686"
    ;;
aarch64 | arm64)
    platform="linux-aarch64"
    ;;
armv6l | armv7l | armv7 | arm)
    platform="linux-armhf"
    ;;
*)
    die "unsupported architecture: $arch"
    ;;
esac

workdir="$(mktemp -d 2>/dev/null || mktemp -d -t salam)"
trap 'rm -rf "$workdir"' EXIT INT TERM
archive="$workdir/download.zip"

if [ -n "$VERSION" ]; then
    ASSET="salam-${VERSION}-${platform}.zip"
    URL="https://github.com/${REPO}/releases/download/v${VERSION}/${ASSET}"
    log "Installing Salam ${VERSION} (${platform}) from:"
    log "  $URL"
    fetch_to_file "$URL" "$archive" || die "download failed: $URL"
else
    log "Resolving latest Salam release with a ${platform} asset..."
    releases_json="$(fetch_to_stdout "https://api.github.com/repos/${REPO}/releases?per_page=10")" ||
        die "could not list releases: https://api.github.com/repos/${REPO}/releases"
    tags="$(printf '%s' "$releases_json" | sed -n 's/.*"tag_name": *"\([^"]*\)".*/\1/p')"
    [ -n "$tags" ] || die "no releases found for ${REPO}"

    found=0
    for tag in $tags; do
        v="${tag#v}"
        asset="salam-${v}-${platform}.zip"
        url="https://github.com/${REPO}/releases/download/${tag}/${asset}"
        log "  trying ${tag}..."
        if fetch_to_file "$url" "$archive" 2>/dev/null && [ -s "$archive" ]; then
            VERSION="$v"
            ASSET="$asset"
            URL="$url"
            found=1
            break
        fi
        rm -f "$archive"
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
