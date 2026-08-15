#!/bin/sh
# Build a distributable release of the Salam toolchain into dist/.

set -e
. "$(dirname "$0")/lib.sh"
CC="${CC:-tcc}"
VERSION="$(cat VERSION 2>/dev/null || echo 0.0.0-dev)"
OS="$(uname -s 2>/dev/null | tr '[:upper:]' '[:lower:]' | sed 's/mingw.*/windows/;s/msys.*/windows/')"
DIST="dist/salam-$VERSION-$OS"
# Manifests are JSON/TOML, so they cannot read VERSION the way the compiler
# and the stdlib now do. Stamping them here keeps every in-repo copy in step
# through one mechanical step rather than a remembered edit - which is how
# they drifted to three different numbers (0.2.2, 0.2.3, 0.2.9) in the first
# place.
stamp_version() { # <file> <sed-expression using $VERSION>
    [ -f "$1" ] || return 0
    sed -i.bak "$2" "$1"
    rm -f "$1.bak"
}
stamp_version "tools/mcp/claude-plugin/.claude-plugin/plugin.json" \
    "s/\"version\": \"[^\"]*\"/\"version\": \"$VERSION\"/"
stamp_version "extensions/vscode/package.json" \
    "s/\"version\": \"[^\"]*\"/\"version\": \"$VERSION\"/"
stamp_version "pyproject.toml" \
    "s/^version = \"[^\"]*\"/version = \"$VERSION\"/"

echo ">> building compiler ($CC)"
sh tools/bash/build-compiler.sh
echo ">> staging $DIST"
rm -rf "$DIST"
mkdir -p "$DIST/bin" "$DIST/doc" "$DIST/tests"
cp salam "$DIST/bin/" 2>/dev/null || cp salam.exe "$DIST/bin/"
cp README.md "$DIST/" 2>/dev/null || true
cp doc/*.md "$DIST/doc/" 2>/dev/null || true
EXAMPLE_DIRS="apps basics data editor-selected features games interop stdlib types webframework"
for lang in en fa; do
    mkdir -p "$DIST/tests/$lang"
    for d in $EXAMPLE_DIRS; do
        cp -r "tests/$lang/$d" "$DIST/tests/$lang/$d" 2>/dev/null || true
    done
done
find "$DIST/tests" \( -name '*.out' -o -name '*.expect' -o -name '*.buildonly' \) -delete 2>/dev/null || true
echo ">> packaging"
(cd dist && tar -czf "salam-$VERSION-$OS.tar.gz" "salam-$VERSION-$OS") 2>/dev/null &&
    echo "   dist/salam-$VERSION-$OS.tar.gz" ||
    echo "   (tar unavailable; staged tree at $DIST)"
echo ">> done. Run:  $DIST/bin/salam version"
