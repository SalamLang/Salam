#!/bin/sh
set -eu
root="$(cd "$(dirname "$0")/../.." && pwd)"
src="$root/ai/frontend/public"
for dir in "$root/editor/assets" "$root/tools/mcp/site"; do
    mkdir -p "$dir"
    for f in icon-192.png icon-512.png apple-touch-icon.png; do
        cp "$src/$f" "$dir/$f"
    done
    cp "$src/logo.svg" "$dir/icon.svg"
done
for dir in "$src" "$root/editor" "$root/tools/mcp/site"; do
    convert "$src/icon-192.png" -define icon:auto-resize=48,32,16 "$dir/favicon.ico"
done
