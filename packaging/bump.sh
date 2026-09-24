#!/usr/bin/env bash

set -euo pipefail

REPO=SalamLang/Salam
HERE=$(cd "$(dirname "$0")" && pwd)
DIST=$HERE/dist

VERSION=${1:-}
SEED_VERSION=${2:-}

[ -n "$VERSION" ] || {
    echo "usage: packaging/bump.sh <version> [seed-version]" >&2
    exit 2
}
VERSION=${VERSION#v}

need() { command -v "$1" >/dev/null 2>&1 || {
    echo "bump.sh: $1 is required" >&2
    exit 1
}; }
need curl
need python3

sha256_of() {
    if command -v sha256sum >/dev/null 2>&1; then
        sha256sum "$1" | cut -d' ' -f1
    else shasum -a 256 "$1" | cut -d' ' -f1; fi
}
sha512_of() {
    if command -v sha512sum >/dev/null 2>&1; then
        sha512sum "$1" | cut -d' ' -f1
    else shasum -a 512 "$1" | cut -d' ' -f1; fi
}

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT INT TERM

echo "==> release v$VERSION"
SUMS=$TMP/SHA256SUMS
curl -fsSL -o "$SUMS" \
    "https://github.com/$REPO/releases/download/v$VERSION/SHA256SUMS" ||
    {
        echo "bump.sh: v$VERSION has no SHA256SUMS asset yet" >&2
        exit 1
    }

sum_for() {
    local name=$1 v
    v=$(awk -v n="$name" '$2 == n || $2 == "*" n { print $1; exit }' "$SUMS")
    [ -n "$v" ] || {
        echo "bump.sh: $name is not listed in SHA256SUMS" >&2
        exit 1
    }
    printf '%s' "$v"
}
sum_opt() {
    awk -v n="$1" '$2 == n || $2 == "*" n { print $1; exit }' "$SUMS"
}

if [ -z "$SEED_VERSION" ]; then
    echo "==> resolving the bootstrap seed release"
    SEED_VERSION=$(curl -fsSL "https://api.github.com/repos/$REPO/releases?per_page=100" |
        python3 -c '
import json, sys
target = sys.argv[1]

def key(v):
    return tuple(int(p) for p in v.split("."))

want = key(target)
best = None
for rel in json.load(sys.stdin):
    tag = rel.get("tag_name", "")
    if not tag.startswith("v") or "-nightly-" in tag or rel.get("draft"):
        continue
    v = tag[1:]
    try:
        k = key(v)
    except ValueError:
        continue
    if k < want and (best is None or k > key(best)):
        best = v
print(best or "")
' "$VERSION")
    [ -n "$SEED_VERSION" ] || {
        echo "bump.sh: could not find a seed release older than $VERSION" >&2
        exit 1
    }
fi
echo "    seed: v$SEED_VERSION"

echo "==> hashing the source tarball and the seed binaries"
SRC_URL="https://github.com/$REPO/archive/refs/tags/v$VERSION.tar.gz"
curl -fsSL -o "$TMP/src.tar.gz" "$SRC_URL"
SHA256_SRC=$(sha256_of "$TMP/src.tar.gz")

seed_sum() {
    local asset=$1 out=$TMP/$1
    if [ ! -f "$out" ]; then
        curl -fsSL -o "$out" "https://github.com/$REPO/releases/download/v$SEED_VERSION/$asset.sha256" \
            2>/dev/null || return 1
    fi
    awk '{ print $1; exit }' "$out"
}

SHA256_SEED_MACOS_ARM64=$(seed_sum "salam-$SEED_VERSION-macos-arm64.tar.gz" || true)
SHA256_SEED_LINUX_X86_64=$(seed_sum "salam-$SEED_VERSION-linux-x86_64.tar.gz" || true)
SHA256_SEED_LINUX_AARCH64=$(seed_sum "salam-$SEED_VERSION-linux-aarch64.tar.gz" || true)

for pair in \
    "macos-arm64:$SHA256_SEED_MACOS_ARM64" \
    "linux-x86_64:$SHA256_SEED_LINUX_X86_64" \
    "linux-aarch64:$SHA256_SEED_LINUX_AARCH64"; do
    [ -n "${pair#*:}" ] || {
        echo "bump.sh: v$SEED_VERSION publishes no checksum for ${pair%%:*}" >&2
        echo "bump.sh: pass an explicit seed version as the second argument" >&2
        exit 1
    }
done

echo "==> hashing the musl tarball for Alpine (sha512)"
MUSL=salam-$VERSION-linux-x86_64-musl.tar.gz
curl -fsSL -o "$TMP/$MUSL" "https://github.com/$REPO/releases/download/v$VERSION/$MUSL"
SHA512_LINUX_MUSL=$(sha512_of "$TMP/$MUSL")

sri() {
    python3 -c 'import base64,binascii,sys; print("sha256-"+base64.b64encode(binascii.unhexlify(sys.argv[1])).decode())' "$1"
}

SHA256_LINUX_X86_64=$(sum_for "salam-$VERSION-linux-x86_64.tar.gz")
SHA256_LINUX_AARCH64=$(sum_for "salam-$VERSION-linux-aarch64.tar.gz")
SHA256_LINUX_ARMHF=$(sum_for "salam-$VERSION-linux-armhf.tar.gz")
SHA256_LINUX_I686=$(sum_opt "salam-$VERSION-linux-i686.tar.gz")
SHA256_MACOS_ARM64=$(sum_for "salam-$VERSION-macos-arm64.tar.gz")
SHA256_WINDOWS_X86_64=$(sum_for "salam-$VERSION-windows-x86_64.zip")
SHA256_WINDOWS_I686=$(sum_for "salam-$VERSION-windows-i686.zip")

RELEASE_DATE=$(curl -fsSL "https://api.github.com/repos/$REPO/releases/tags/v$VERSION" |
    python3 -c 'import json,sys; print((json.load(sys.stdin).get("published_at") or "")[:10])')
[ -n "$RELEASE_DATE" ] || RELEASE_DATE=$(date -u +%Y-%m-%d)
RPM_DATE=$(LC_ALL=C date -u -d "$RELEASE_DATE" "+%a %b %d %Y" 2>/dev/null ||
    LC_ALL=C date -u -j -f %Y-%m-%d "$RELEASE_DATE" "+%a %b %d %Y")

render() {
    local src=$1 dst=$2 upper=${3:-}
    local s256_x64=$SHA256_WINDOWS_X86_64 s256_x86=$SHA256_WINDOWS_I686
    if [ "$upper" = upper ]; then
        s256_x64=$(printf '%s' "$s256_x64" | tr 'a-f' 'A-F')
        s256_x86=$(printf '%s' "$s256_x86" | tr 'a-f' 'A-F')
    fi
    mkdir -p "$(dirname "$dst")"
    sed \
        -e "s|@VERSION@|$VERSION|g" \
        -e "s|@SEED_VERSION@|$SEED_VERSION|g" \
        -e "s|@RELEASE_DATE@|$RELEASE_DATE|g" \
        -e "s|@RPM_DATE@|$RPM_DATE|g" \
        -e "s|@SHA256_SRC@|$SHA256_SRC|g" \
        -e "s|@SHA256_LINUX_X86_64@|$SHA256_LINUX_X86_64|g" \
        -e "s|@SHA256_LINUX_AARCH64@|$SHA256_LINUX_AARCH64|g" \
        -e "s|@SHA256_LINUX_ARMHF@|$SHA256_LINUX_ARMHF|g" \
        -e "s|@SHA256_LINUX_I686@|$SHA256_LINUX_I686|g" \
        -e "s|@SHA256_MACOS_ARM64@|$SHA256_MACOS_ARM64|g" \
        -e "s|@SHA256_WINDOWS_X86_64@|$s256_x64|g" \
        -e "s|@SHA256_WINDOWS_I686@|$s256_x86|g" \
        -e "s|@SHA256_SEED_MACOS_ARM64@|$SHA256_SEED_MACOS_ARM64|g" \
        -e "s|@SHA256_SEED_LINUX_X86_64@|$SHA256_SEED_LINUX_X86_64|g" \
        -e "s|@SHA256_SEED_LINUX_AARCH64@|$SHA256_SEED_LINUX_AARCH64|g" \
        -e "s|@SHA512_LINUX_MUSL@|$SHA512_LINUX_MUSL|g" \
        -e "s|@SRI_LINUX_X86_64@|$(sri "$SHA256_LINUX_X86_64")|g" \
        -e "s|@SRI_LINUX_AARCH64@|$(sri "$SHA256_LINUX_AARCH64")|g" \
        -e "s|@SRI_MACOS_ARM64@|$(sri "$SHA256_MACOS_ARM64")|g" \
        "$src" >"$dst"
}

echo "==> rendering into packaging/dist"
rm -rf "$DIST"

render "$HERE/homebrew/salam.rb" "$DIST/homebrew/salam.rb"
render "$HERE/aur/salam-bin/PKGBUILD" "$DIST/aur/salam-bin/PKGBUILD"
render "$HERE/aur/salamlang/PKGBUILD"    "$DIST/aur/salamlang/PKGBUILD"
render "$HERE/rpm/salam.spec" "$DIST/rpm/salam.spec"
render "$HERE/alpine/APKBUILD" "$DIST/alpine/APKBUILD"
render "$HERE/nix/package.nix" "$DIST/nix/package.nix"
cp "$HERE/nix/flake.nix" "$DIST/nix/flake.nix"
render "$HERE/snap/snapcraft.yaml" "$DIST/snap/snapcraft.yaml"
render "$HERE/scoop/salam.json" "$DIST/scoop/salam.json"
render "$HERE/winget/SalamLang.Salam.yaml" "$DIST/winget/SalamLang.Salam.yaml" upper
render "$HERE/winget/SalamLang.Salam.installer.yaml" "$DIST/winget/SalamLang.Salam.installer.yaml" upper
render "$HERE/winget/SalamLang.Salam.locale.en-US.yaml" "$DIST/winget/SalamLang.Salam.locale.en-US.yaml" upper
render "$HERE/chocolatey/salam.nuspec" "$DIST/chocolatey/salam.nuspec"
render "$HERE/chocolatey/tools/chocolateyinstall.ps1" "$DIST/chocolatey/tools/chocolateyinstall.ps1"
cp "$HERE/chocolatey/tools/chocolateyuninstall.ps1" "$DIST/chocolatey/tools/chocolateyuninstall.ps1"
render "$HERE/npm/package.json" "$DIST/npm/package.json"
cp "$HERE/npm/install.js" "$DIST/npm/install.js"
mkdir -p "$DIST/npm/bin" && cp "$HERE/npm/bin/salam.js" "$DIST/npm/bin/salam.js"
mkdir -p "$DIST/asdf" && cp -R "$HERE/asdf/bin" "$DIST/asdf/bin"

leftover=$(grep -rl '@[A-Z0-9_]\{3,\}@' "$DIST" || true)
if [ -n "$leftover" ]; then
    echo "bump.sh: unsubstituted placeholders remain:" >&2
    grep -rno '@[A-Z0-9_]\{3,\}@' $leftover >&2
    exit 1
fi

echo
echo "rendered packaging/dist for v$VERSION (seed v$SEED_VERSION):"
find "$DIST" -type f | sed "s|$HERE/|  packaging/|"
