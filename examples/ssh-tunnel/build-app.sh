#!/bin/sh
set -eu

here=$(cd "$(dirname "$0")" && pwd)
salam=${SALAM:-salam}
app="${1:-$HOME/Applications}/Salam Tunnel.app"

std=${SALAM_STD:-$here/../../std}
musl=${SALAM_MUSL_SYSROOT:-$HOME/.salam/sysroots/x86_64-linux-musl}

mkdir -p "$app/Contents/MacOS" "$app/Contents/Resources"
"$salam" build "$here" --ui=native --stdlib-path="$std" -o "$app/Contents/MacOS/salam-tunnel"
if [ -f "$musl/libc.a" ]; then
    SALAM_MUSL_SYSROOT="$musl" "$salam" build "$here/relay/main.salam" \
        --target=x86_64-linux-musl --backend=native --linker=builtin \
        --stdlib-path="$std" -o "$app/Contents/Resources/salam-udprelay-linux-x86_64"
else
    echo "No musl sysroot at $musl: building without the UDP relay" >&2
    rm -f "$app/Contents/Resources/salam-udprelay-linux-x86_64"
fi
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
