#!/bin/sh
# Salam installer for Linux, macOS and BSD.
#
# Usage:
#   curl -fsSL https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.sh | sh
#   curl -fsSL https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.sh | sh -s -- --dir .
#

set -eu

REPO="SalamLang/Salam"
INSTALL_DIR="${SALAM_INSTALL_DIR:-}"
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
        sed -n '2,13p' "$0"
        exit 0
        ;;
    *) die "unknown option: $1" ;;
    esac
done

fetch_to_stdout() {
    # $1 = URL
    if command -v curl >/dev/null 2>&1; then
        curl -fsSL "$1"
    elif command -v wget >/dev/null 2>&1; then
        wget -qO- "$1"
    else
        die "need curl or wget to install Salam"
    fi
}

fetch_to_file() {
    # $1 = URL, $2 = output path
    if command -v curl >/dev/null 2>&1; then
        curl -fL --retry 3 --retry-delay 2 -o "$2" "$1"
    elif command -v wget >/dev/null 2>&1; then
        wget -q --tries=3 -O "$2" "$1"
    else
        die "need curl or wget to install Salam"
    fi
}

os="$(uname -s)"
case "$os" in
Linux)
    kernel="linux"
    ;;
Darwin)
    kernel="mac"
    ;;
FreeBSD | OpenBSD | NetBSD | DragonFly)
    die "Salam does not publish native $os binaries yet. Linux and macOS binaries are ELF/Mach-O and will not run here." \
        "Follow BUILD instructions in the repository README to compile from source: https://github.com/${REPO}"
    ;;
MINGW* | MSYS* | CYGWIN*)
    die "this is a Windows shell; use install.bat instead: https://raw.githubusercontent.com/${REPO}/refs/heads/main/install.bat"
    ;;
*)
    die "unsupported OS: $os"
    ;;
esac

arch="$(uname -m)"
platform=""
if [ "$kernel" = "mac" ]; then
    platform="mac"
else
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
        die "unsupported architecture: $arch ($kernel)"
        ;;
    esac
fi

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
elif command -v python3 >/dev/null 2>&1; then
    python3 -c 'import sys,zipfile; zipfile.ZipFile(sys.argv[1]).extractall(sys.argv[2])' "$archive" "$extract_dir"
elif command -v python >/dev/null 2>&1; then
    python -c 'import sys,zipfile; zipfile.ZipFile(sys.argv[1]).extractall(sys.argv[2])' "$archive" "$extract_dir"
else
    die "need unzip, bsdtar or python to extract the release archive"
fi

binary="$extract_dir/salam-${platform}/salam"
[ -f "$binary" ] || binary="$(find "$extract_dir" -type f -name salam | head -n 1)"
[ -f "$binary" ] || die "could not find 'salam' binary inside $ASSET"

if [ -z "$INSTALL_DIR" ]; then
    INSTALL_DIR="$HOME/.salam/bin"
fi
mkdir -p "$INSTALL_DIR"
cp "$binary" "$INSTALL_DIR/salam"
chmod +x "$INSTALL_DIR/salam"

log "Installed: $INSTALL_DIR/salam"
"$INSTALL_DIR/salam" version >&2 2>/dev/null || true

if [ "$INSTALL_DIR" = "$HOME/.salam/bin" ]; then
    case ":$PATH:" in
    *":$INSTALL_DIR:"*) ;;
    *)
        rc=""
        case "${SHELL:-}" in
        */zsh) rc="$HOME/.zshrc" ;;
        */bash) [ -f "$HOME/.bash_profile" ] && rc="$HOME/.bash_profile" || rc="$HOME/.bashrc" ;;
        *) rc="$HOME/.profile" ;;
        esac
        line="export PATH=\"$INSTALL_DIR:\$PATH\""
        if [ -f "$rc" ] && grep -qF "$INSTALL_DIR" "$rc" 2>/dev/null; then
            :
        else
            printf '\n# added by Salam installer\n%s\n' "$line" >>"$rc"
            log "Added $INSTALL_DIR to PATH in $rc (restart your shell, or run: $line)"
        fi
        ;;
    esac
fi

log "Done. Run 'salam version' to verify (open a new shell first if PATH was just updated)."
