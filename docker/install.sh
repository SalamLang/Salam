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
#   sh install.sh --dir /opt/salam --version 0.2.9
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

# GitHub's REST API budgets anonymous callers per source IP, and shared CI
# runners exhaust that budget between them - the call then answers 403 for
# everyone on that IP. A token moves it onto the far larger authenticated
# budget; GitHub Actions exposes one as github.token. Only ever sent to
# api.github.com so it cannot leak to a release download or a redirect.
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

# Newest release tag first, one per line. Empty output means neither source
# answered.
list_release_tags() {
    json="$(fetch_to_stdout "https://api.github.com/repos/${REPO}/releases?per_page=10" 2>/dev/null || true)"
    found_tags="$(printf '%s' "$json" | sed -n 's/.*"tag_name": *"\([^"]*\)".*/\1/p')"
    if [ -z "$found_tags" ]; then
        # The API is rate-limited and unauthenticated CI runners routinely
        # hit 403 on it. The releases feed is served by github.com itself
        # and carries no such limit, so it keeps the installer working.
        log "  releases API unavailable, falling back to the Atom feed"
        atom="$(fetch_to_stdout "https://github.com/${REPO}/releases.atom" 2>/dev/null || true)"
        found_tags="$(printf '%s' "$atom" | sed -n 's#.*/releases/tag/\([^"]*\)".*#\1#p')"
    fi
    # Nightlies are prereleases and their assets carry the plain version
    # (salam-0.2.9-linux.zip under a v0.2.9-nightly-<date> tag), so they can
    # never match the name built from the tag below - dropping them here just
    # skips a guaranteed-failed download attempt. Filtered by name because
    # the Atom fallback carries no prerelease flag to filter on.
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
    tags="$(list_release_tags)"
    lookup_err="could not list releases for ${REPO}: both"
    lookup_err="$lookup_err https://api.github.com/repos/${REPO}/releases"
    lookup_err="$lookup_err and https://github.com/${REPO}/releases.atom failed."
    lookup_err="$lookup_err Pass --version to skip the lookup."
    [ -n "$tags" ] || die "$lookup_err"

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
