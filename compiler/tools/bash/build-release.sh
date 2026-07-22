#!/bin/sh
# Build a distributable release of the Salam toolchain into dist/.

set -e
. "$(dirname "$0")/lib.sh"
CC="${CC:-tcc}"
VERSION="$(cat ../VERSION 2>/dev/null || echo 0.2.5)"
OS="$(uname -s 2>/dev/null | tr '[:upper:]' '[:lower:]' | sed 's/mingw.*/windows/;s/msys.*/windows/')"
DIST="dist/salam-$VERSION-$OS"
echo ">> building compiler ($CC)"
sh tools/bash/build-compiler.sh
echo ">> staging $DIST"
rm -rf "$DIST"
mkdir -p "$DIST/bin" "$DIST/doc" "$DIST/examples"
cp salam "$DIST/bin/" 2>/dev/null || cp salam.exe "$DIST/bin/"
cp ../README.md "$DIST/" 2>/dev/null || true
cp doc/*.md "$DIST/doc/" 2>/dev/null || true
cp -r ../examples/en ../examples/fa "$DIST/examples/" 2>/dev/null || true
echo ">> packaging"
(cd dist && tar -czf "salam-$VERSION-$OS.tar.gz" "salam-$VERSION-$OS") 2>/dev/null &&
    echo "   dist/salam-$VERSION-$OS.tar.gz" ||
    echo "   (tar unavailable; staged tree at $DIST)"
echo ">> done. Run:  $DIST/bin/salam version"
