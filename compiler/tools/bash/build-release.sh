#!/bin/sh
# Build a distributable release of the Salam toolchain into dist/.
# Self-hosted: rebuilds compiler/salam from compiler/main.salam via the
# current compiler/salam (there is no C source to build from anymore).

set -e
. "$(dirname "$0")/lib.sh" # anchors cwd at the repo root
VERSION="$(cat VERSION 2>/dev/null || echo 0.2.6)"
OS="$(uname -s 2>/dev/null | tr '[:upper:]' '[:lower:]' | sed 's/mingw.*/windows/;s/msys.*/windows/')"
DIST="dist/salam-$VERSION-$OS"
echo ">> building compiler (self-hosted)"
sh compiler/tools/bash/build-compiler.sh
echo ">> staging $DIST"
rm -rf "$DIST"
# bin/ + share/salam/std/ matches resolve_stdlib_root's installed-prefix
# fallback ("../share/salam" relative to the binary), so `bin/salam` finds
# its stdlib automatically with no SALAM_STD/salam.cfg needed.
mkdir -p "$DIST/bin" "$DIST/share/salam/std" "$DIST/examples"
cp compiler/salam "$DIST/bin/" 2>/dev/null || cp compiler/salam.exe "$DIST/bin/"
cp README.md "$DIST/" 2>/dev/null || true
cp -r std/* "$DIST/share/salam/std/" 2>/dev/null || true
EXAMPLE_DIRS="apps basics data editor-selected features games interop stdlib types webframework"
for lang in en fa; do
    mkdir -p "$DIST/examples/$lang"
    for d in $EXAMPLE_DIRS; do
        cp -r "tests/$lang/$d" "$DIST/examples/$lang/$d" 2>/dev/null || true
    done
done
find "$DIST/examples" \( -name '*.out' -o -name '*.expect' -o -name '*.buildonly' \) -delete 2>/dev/null || true
echo ">> packaging"
(cd dist && tar -czf "salam-$VERSION-$OS.tar.gz" "salam-$VERSION-$OS") 2>/dev/null &&
    echo "   dist/salam-$VERSION-$OS.tar.gz" ||
    echo "   (tar unavailable; staged tree at $DIST)"
echo ">> done. Run:  $DIST/bin/salam version"
