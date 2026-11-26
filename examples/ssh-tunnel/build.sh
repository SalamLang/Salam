#!/bin/sh
set -eu

here=$(cd "$(dirname "$0")" && pwd)
salam=${SALAM:-salam}
std=${SALAM_STD:-$here/../../std}
case $(uname -s) in
Darwin) host=macos ;;
Linux) host=linux ;;
*) host=windows ;;
esac
platform=${1:-$host}

relays() {
    for arch in x86_64 aarch64; do
        musl=${SALAM_SYSROOTS:-$HOME/.salam/sysroots}/$arch-linux-musl
        out="$1/salam-udprelay-linux-$arch"
        if [ -f "$musl/libc.a" ]; then
            SALAM_MUSL_SYSROOT="$musl" "$salam" build "$here/relay/main.salam" \
                --target="$arch-linux-musl" --backend=native --linker=builtin \
                --stdlib-path="$std" -o "$out"
        elif ! "$salam" build "$here/relay/main.salam" --target="$arch-linux-musl" \
            --backend=native --linker=builtin --stdlib-path="$std" -o "$out"; then
            echo "No musl toolchain for $arch: UDP will not work with such servers" >&2
        fi
    done
}

wintun() {
    version=0.14.1
    sum=07c256185d6ee3652e09fa55c0b673e2624b565e02c4b9091c79ca7d2f24ef51
    work=$(mktemp -d)
    curl -fsSL -A salam-build -o "$work/wintun.zip" \
        "https://www.wintun.net/builds/wintun-$version.zip"
    if command -v sha256sum >/dev/null 2>&1; then
        got=$(sha256sum "$work/wintun.zip" | cut -d' ' -f1)
    else
        got=$(shasum -a 256 "$work/wintun.zip" | cut -d' ' -f1)
    fi
    if [ "$got" != "$sum" ]; then
        echo "The wintun download does not match its checksum" >&2
        exit 1
    fi
    unzip -p "$work/wintun.zip" wintun/bin/amd64/wintun.dll >"$1/wintun.dll"
    unzip -p "$work/wintun.zip" wintun/LICENSE.txt >"$1/wintun-LICENSE.txt"
    rm -r "${work:?}"
}

case $platform in
macos)
    app="${2:-$HOME/Applications}/Salam Tunnel.app"
    mkdir -p "$app/Contents/MacOS" "$app/Contents/Resources"
    "$salam" build "$here" --ui=native --stdlib-path="$std" -o "$app/Contents/MacOS/salam-tunnel"
    relays "$app/Contents/Resources"
    cat > "$app/Contents/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>salam-tunnel</string>
    <key>CFBundleIdentifier</key>
    <string>ir.salamlang.sshtunnel</string>
    <key>CFBundleName</key>
    <string>Salam Tunnel</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0.0</string>
    <key>NSHighResolutionCapable</key>
    <true/>
</dict>
</plist>
PLIST
    codesign --force --sign - "$app" >/dev/null 2>&1 || true
    echo "Built $app"
    ;;
linux)
    dir="${2:-$here/dist/linux}"
    mkdir -p "$dir"
    "$salam" build "$here" --ui=native --backend=native --linker=system --stdlib-path="$std" \
        -o "$dir/salam-tunnel"
    relays "$dir"
    echo "Built $dir/salam-tunnel"
    ;;
windows)
    dir="${2:-$here/dist/windows}"
    mkdir -p "$dir"
    "$salam" build "$here" --ui=native --target=x86_64-w64-windows-gnu --backend=native \
        --subsystem=windows \
        --stdlib-path="$std" -o "$dir/salam-tunnel.exe"
    relays "$dir"
    wintun "$dir"
    echo "Built $dir/salam-tunnel.exe"
    ;;
*)
    echo "usage: build.sh [macos|linux|windows] [folder]" >&2
    exit 2
    ;;
esac
