#!/usr/bin/env bash

set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)

VERSION=
INPUT=
OUTPUT=
while [ $# -gt 0 ]; do
    case $1 in
    --version) VERSION=${2#v}; shift 2 ;;
    --input) INPUT=$2; shift 2 ;;
    --output) OUTPUT=$2; shift 2 ;;
    *) echo "usage: build-pacman.sh --version V --input DIR --output DIR" >&2; exit 2 ;;
    esac
done
[ -n "$VERSION" ] && [ -n "$INPUT" ] && [ -n "$OUTPUT" ] || {
    echo "usage: build-pacman.sh --version V --input DIR --output DIR" >&2
    exit 2
}

INPUT=$(cd "$INPUT" && pwd)
mkdir -p "$OUTPUT"
OUTPUT=$(cd "$OUTPUT" && pwd)

X64=$INPUT/salam-$VERSION-linux-x86_64.tar.gz
A64=$INPUT/salam-$VERSION-linux-aarch64.tar.gz
[ -f "$X64" ] || { echo "build-pacman.sh: $X64 is missing" >&2; exit 1; }

sum_or_skip() { if [ -f "$1" ]; then sha256sum "$1" | cut -d' ' -f1; else echo SKIP; fi; }

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT INT TERM
sed \
    -e "s|@VERSION@|$VERSION|g" \
    -e "s|@SHA256_LINUX_X86_64@|$(sum_or_skip "$X64")|g" \
    -e "s|@SHA256_LINUX_AARCH64@|$(sum_or_skip "$A64")|g" \
    -e 's|^_url=.*|_url="file:///in"|' \
    "$HERE/../aur/salam-bin/PKGBUILD" >"$WORK/PKGBUILD"

cat >"$WORK/build.sh" <<'INNER'
set -eu
useradd -m builder
install -d -o builder -g builder /home/builder/w
cp /in/PKGBUILD /home/builder/w/PKGBUILD
chown builder:builder /home/builder/w/PKGBUILD
su builder -c "cd ~/w && makepkg -f --noconfirm"
if [ -f /in/salam-$VERSION-linux-aarch64.tar.gz ]; then
    su builder -c "cd ~/w && CARCH=aarch64 makepkg -f --noconfirm"
fi
mkdir -p /out
for f in /home/builder/w/salam-bin-*.pkg.tar.zst; do
    arch=${f##*-}
    arch=${arch%.pkg.tar.zst}
    cp "$f" "/out/salam-$VERSION-linux-$arch.pkg.tar.zst"
done
INNER

CID=$(docker create -e VERSION="$VERSION" archlinux:base-devel bash /in/build.sh)
trap 'docker rm -f "$CID" >/dev/null 2>&1; rm -rf "$WORK"' EXIT INT TERM
mkdir -p "$WORK/in"
cp "$WORK/PKGBUILD" "$WORK/build.sh" "$WORK/in/"
cp "$X64" "$WORK/in/"
[ -f "$A64" ] && cp "$A64" "$WORK/in/"
docker cp "$WORK/in/." "$CID:/in"
docker start -a "$CID"
docker cp "$CID:/out/." "$OUTPUT"
