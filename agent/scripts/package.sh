#!/usr/bin/env bash
# Collect one target's Tauri bundles under release names and add portable
# .tar.gz and .zip archives.
# Usage: package.sh <linux|macos|windows> <arch> <rust-target> <version> <out-dir>
set -euo pipefail
shopt -s nullglob

[ $# -eq 5 ] || {
    echo "usage: $0 <os> <arch> <rust-target> <version> <out-dir>" >&2
    exit 2
}
os=$1 arch=$2 target=$3 version=$4
mkdir -p "$5"
out=$(cd "$5" && pwd)

app_dir=$(cd "$(dirname "$0")/.." && pwd)
repo=$(cd "$app_dir/.." && pwd)
rel="$app_dir/src-tauri/target/$target/release"
bundle="$rel/bundle"
name="salam-agent-$version-$os-$arch"

die() {
    echo "::error::$*" >&2
    exit 1
}

pick() {
    local matches=("$@")
    [ ${#matches[@]} -eq 1 ] || die "expected one file, found ${#matches[@]}: ${matches[*]:-none}"
    printf '%s' "${matches[0]}"
}

stage=$(mktemp -d)
trap 'rm -rf "$stage"' EXIT
mkdir "$stage/$name"
cp "$repo/LICENSE" "$stage/$name/LICENSE"

case "$os" in
linux)
    cp "$(pick "$bundle"/deb/*.deb)" "$out/$name.deb"
    cp "$(pick "$bundle"/rpm/*.rpm)" "$out/$name.rpm"
    appimages=("$bundle"/appimage/*.AppImage)
    if [ ${#appimages[@]} -gt 0 ]; then
        cp "$(pick "${appimages[@]}")" "$out/$name.AppImage"
    fi
    cp "$rel/salam-agent" "$stage/$name/salam-agent"
    chmod 755 "$stage/$name/salam-agent"
    ;;
windows)
    cp "$(pick "$bundle"/nsis/*-setup.exe)" "$out/$name-setup.exe"
    msi_files=("$bundle"/msi/*.msi)
    if [ ${#msi_files[@]} -gt 0 ]; then
        cp "$(pick "${msi_files[@]}")" "$out/$name.msi"
    fi
    cp "$rel/salam-agent.exe" "$stage/$name/salam-agent.exe"
    ;;
macos)
    cp "$(pick "$bundle"/dmg/*.dmg)" "$out/$name.dmg"
    cp -R "$(pick "$bundle"/macos/*.app)" "$stage/$name/"
    ;;
*)
    die "unknown os: $os"
    ;;
esac

(cd "$stage" && tar -czf "$out/$name.tar.gz" "$name")
if command -v zip >/dev/null 2>&1; then
    (cd "$stage" && zip -qry "$out/$name.zip" "$name")
elif command -v 7z >/dev/null 2>&1; then
    (cd "$stage" && 7z a -tzip -bso0 -bsp0 "$out/$name.zip" "$name")
else
    die "neither zip nor 7z is installed"
fi

ls -la "$out"
