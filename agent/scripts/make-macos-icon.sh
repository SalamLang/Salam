#!/usr/bin/env bash
# Build the macOS app icon from design/logo/logo-box-macos.svg.
#
# The master SVG follows Apple's macOS icon grid: a 1024x1024 canvas with the
# 824x824 rounded body centred, so Dock and Finder show it at the same size as
# native apps. The generated icns replaces src-tauri/icons/icon.icns, and
# icon-macos.png is the image the app sets as its Dock icon when it runs outside
# the .app bundle. Requires rsvg-convert, sips and iconutil (macOS).
set -euo pipefail

here=$(cd "$(dirname "$0")" && pwd)
icons="$here/../src-tauri/icons"
source_svg=$(cd "$here/../.." && pwd)/design/logo/logo-box-macos.svg

command -v rsvg-convert >/dev/null || {
    echo "rsvg-convert is required" >&2
    exit 1
}

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
mkdir "$tmp/Salam.iconset"

rsvg-convert -w 1024 -h 1024 "$source_svg" -o "$icons/icon-macos.png"
for size in 16 32 64 128 256 512 1024; do
    sips -z "$size" "$size" "$icons/icon-macos.png" --out "$tmp/$size.png" >/dev/null
done

cp "$tmp/16.png" "$tmp/Salam.iconset/icon_16x16.png"
cp "$tmp/32.png" "$tmp/Salam.iconset/icon_16x16@2x.png"
cp "$tmp/32.png" "$tmp/Salam.iconset/icon_32x32.png"
cp "$tmp/64.png" "$tmp/Salam.iconset/icon_32x32@2x.png"
cp "$tmp/128.png" "$tmp/Salam.iconset/icon_128x128.png"
cp "$tmp/256.png" "$tmp/Salam.iconset/icon_128x128@2x.png"
cp "$tmp/256.png" "$tmp/Salam.iconset/icon_256x256.png"
cp "$tmp/512.png" "$tmp/Salam.iconset/icon_256x256@2x.png"
cp "$tmp/512.png" "$tmp/Salam.iconset/icon_512x512.png"
cp "$tmp/1024.png" "$tmp/Salam.iconset/icon_512x512@2x.png"

iconutil -c icns "$tmp/Salam.iconset" -o "$icons/icon.icns"
echo "wrote $icons/icon-macos.png and $icons/icon.icns"
