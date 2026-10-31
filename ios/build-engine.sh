#!/bin/sh
# Prepares everything the Xcode project compiles or bundles besides the Swift
# sources: the engine's generated C, a minified std/, the examples and a CA
# bundle for TLS. Run it before `xcodegen generate`.
#
# Usage: ios/build-engine.sh

set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/.." && pwd)
OUT="$HERE/build"
mkdir -p "$OUT"

sh "$ROOT/tools/bash/build-mobile-engine.sh" --out "$OUT/engine" --target arm64-apple-ios --std "$OUT/std"

rm -rf "$OUT/examples"
mkdir -p "$OUT/examples/en" "$OUT/examples/fa"
cp "$ROOT/tools/salam/editor-examples.tsv" "$OUT/examples/index.tsv"
for lang in en fa; do
    for f in "$ROOT"/tests/"$lang"/editor-selected/*.salam; do
        [ -f "$f" ] || continue
        awk 'NR == 1 && /^\/\*/ { banner = 1 }
            banner { if (/\*\//) { banner = 0; gap = 1 } next }
            gap && /^[[:space:]]*$/ { next }
            { gap = 0; print }' "$f" >"$OUT/examples/$lang/$(basename "$f" | sed 's/^[0-9]*-//')"
    done
done

if [ ! -s "$OUT/cacert.pem" ]; then
    curl -fsSL --retry 3 -o "$OUT/cacert.pem" https://curl.se/ca/cacert.pem
fi
echo "ios engine inputs ready in $OUT"
