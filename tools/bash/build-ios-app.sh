#!/bin/sh
# Builds a page or program with the native UI into an iOS app bundle.
#
# Usage:
#   tools/bash/build-ios-app.sh APP.salam --out DIR [--device] [--id BUNDLE_ID] [--name LABEL]
#       [--icon ICON.png]
#       [--permission NAME]...
#
# Salam emits C for arm64, clang builds it with the host shim in
# mobile/host/ios, and DIR/App.app is the result. The default target is the
# simulator; --device builds for a phone and still needs signing in Xcode.
#
# --icon ICON.png (one square PNG, 1024 pixels is best) becomes the app icon.
# It goes through an asset catalog compiled with actool; without actool the
# icon sizes are written as loose PNG files that Info.plist names.
#
# --permission NAME (camera, microphone, location, notifications) adds the
# usage description the system shows in its permission dialog, so
# device.RequestPermission can ask for it. Calls that need a description
# themselves, such as device.TakePhoto or device.RequestLocation, add it alone.

set -eu
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
. "$ROOT/mobile/host/common/icons.sh"

: "${SALAM:=salam}"
APP=
OUT=
DEVICE=0
ID=
NAME=
PERMS=
ICON=
while [ $# -gt 0 ]; do
    case $1 in
    --out)
        OUT=$2
        shift 2
        ;;
    --device)
        DEVICE=1
        shift
        ;;
    --id)
        ID=$2
        shift 2
        ;;
    --name)
        NAME=$2
        shift 2
        ;;
    --icon)
        ICON=$2
        shift 2
        ;;
    --permission)
        PERMS="$PERMS $2"
        shift 2
        ;;
    -*)
        echo "build-ios-app: unknown argument '$1'" >&2
        exit 2
        ;;
    *)
        APP=$1
        shift
        ;;
    esac
done
[ -n "$APP" ] && [ -n "$OUT" ] || {
    echo "usage: build-ios-app.sh APP.salam --out DIR [--device] [--id ID] [--name LABEL] [--icon ICON.png] [--permission NAME]..." >&2
    exit 2
}
[ -f "$APP" ] || {
    echo "build-ios-app: no such file: $APP" >&2
    exit 2
}
if [ -n "$ICON" ]; then
    icon_check "$ICON" build-ios-app || exit 2
    ICON=$(cd "$(dirname "$ICON")" && pwd)/$(basename "$ICON")
fi

APP=$(cd "$(dirname "$APP")" && pwd)/$(basename "$APP")
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)
base=$(basename "$APP" .salam)
[ -n "$ID" ] || ID=app.salam.$(printf %s "$base" | tr -c 'A-Za-z0-9\n' '_' | tr '[:upper:]' '[:lower:]')
[ -n "$NAME" ] || NAME=$base

if [ "$DEVICE" = 1 ]; then
    SDK=iphoneos
    TRIPLE=arm64-apple-ios15.0
else
    SDK=iphonesimulator
    TRIPLE=arm64-apple-ios15.0-simulator
fi
command -v xcrun >/dev/null 2>&1 || {
    echo "build-ios-app: xcrun not found; iOS apps need Xcode on macOS (install Xcode, then run: xcode-select -s /Applications/Xcode.app)" >&2
    exit 1
}
SYSROOT=$(xcrun --sdk "$SDK" --show-sdk-path 2>/dev/null) || {
    echo "build-ios-app: no $SDK SDK; install Xcode and select it with: xcode-select -s /Applications/Xcode.app" >&2
    exit 1
}
CC="xcrun --sdk $SDK clang -target $TRIPLE -isysroot $SYSROOT"

work="$OUT/work"
rm -rf "$work" "$OUT/App.app"
mkdir -p "$work/obj" "$OUT/App.app"
(
    cd "$work"
    "$SALAM" build "$APP" --ui=native --backend=c --emit-c --target="$TRIPLE" \
        --output="$work/app" --log-level=warn
)
for f in "$work"/.salam-build/*.c; do
    $CC -O2 -w -fno-strict-aliasing -fno-finite-loops -Dmain=salam_app_main \
        -I"$work/.salam-build" -c "$f" -o "$work/obj/$(basename "$f").o"
done
$CC -O2 -fobjc-arc -c "$ROOT/mobile/host/ios/salam_host.m" -o "$work/obj/salam_host_ios.o"
$CC -O2 -c "$ROOT/mobile/host/common/salam_host_common.c" -o "$work/obj/salam_host_common.o"
$CC -o "$OUT/App.app/App" "$work"/obj/*.o -lm \
    -framework UIKit -framework Foundation -framework CoreGraphics -framework QuartzCore \
    -framework AVFoundation -framework AudioToolbox -framework CoreLocation -framework PhotosUI \
    -framework UserNotifications

uses_host_call() {
    grep -qsw "$1" "$work"/.salam-build/*.c
}
USAGE_KEYS=
add_usage() {
    case $USAGE_KEYS in
    *"<key>$1</key>"*) return 0 ;;
    esac
    USAGE_KEYS="$USAGE_KEYS
    <key>$1</key><string>$2</string>"
}
if uses_host_call salam_host_location_request; then
    add_usage NSLocationWhenInUseUsageDescription "$NAME uses your location."
fi
if uses_host_call salam_host_photo_request; then
    add_usage NSCameraUsageDescription "$NAME uses the camera to take photos."
    add_usage NSPhotoLibraryUsageDescription "$NAME reads photos you choose."
fi
for name in $PERMS; do
    case $name in
    camera) add_usage NSCameraUsageDescription "$NAME uses the camera." ;;
    microphone) add_usage NSMicrophoneUsageDescription "$NAME uses the microphone." ;;
    location) add_usage NSLocationWhenInUseUsageDescription "$NAME uses your location." ;;
    notifications) ;;
    *)
        echo "build-ios-app: unknown permission '$name' (camera, microphone, location, notifications)" >&2
        exit 2
        ;;
    esac
done

cat >"$OUT/App.app/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key><string>App</string>
    <key>CFBundleIdentifier</key><string>$ID</string>
    <key>CFBundleName</key><string>$NAME</string>
    <key>CFBundleDisplayName</key><string>$NAME</string>
    <key>CFBundlePackageType</key><string>APPL</string>
    <key>CFBundleShortVersionString</key><string>1.0</string>
    <key>CFBundleVersion</key><string>1</string>
    <key>MinimumOSVersion</key><string>15.0</string>
    <key>UILaunchScreen</key><dict/>$USAGE_KEYS
    <key>UIApplicationSceneManifest</key>
    <dict>
        <key>UIApplicationSupportsMultipleScenes</key><false/>
        <key>UISceneConfigurations</key>
        <dict>
            <key>UIWindowSceneSessionRoleApplication</key>
            <array>
                <dict>
                    <key>UISceneConfigurationName</key><string>Default</string>
                    <key>UISceneDelegateClassName</key><string>SalamSceneDelegate</string>
                </dict>
            </array>
        </dict>
    </dict>
    <key>UIRequiredDeviceCapabilities</key><array><string>arm64</string></array>
    <key>UISupportedInterfaceOrientations</key>
    <array>
        <string>UIInterfaceOrientationPortrait</string>
        <string>UIInterfaceOrientationLandscapeLeft</string>
        <string>UIInterfaceOrientationLandscapeRight</string>
    </array>
</dict>
</plist>
PLIST
if [ -n "$ICON" ]; then
    cat_dir=$work/icon/Assets.xcassets
    mkdir -p "$cat_dir/AppIcon.appiconset"
    cp "$ICON" "$cat_dir/AppIcon.appiconset/icon.png"
    printf '%s\n' '{"info":{"author":"xcode","version":1}}' >"$cat_dir/Contents.json"
    printf '%s\n' '{"images":[{"filename":"icon.png","idiom":"universal","platform":"ios","size":"1024x1024"}],"info":{"author":"xcode","version":1}}' \
        >"$cat_dir/AppIcon.appiconset/Contents.json"
    if xcrun actool --compile "$OUT/App.app" --platform "$SDK" --minimum-deployment-target 15.0 \
        --app-icon AppIcon --target-device iphone --target-device ipad \
        --output-partial-info-plist "$work/icon/partial.plist" "$cat_dir" >"$work/icon/actool.log" 2>&1 &&
        [ -f "$work/icon/partial.plist" ]; then
        /usr/libexec/PlistBuddy -c "Merge $work/icon/partial.plist" "$OUT/App.app/Info.plist"
    else
        echo "build-ios-app: actool failed, writing loose icon files" >&2
        icon_resize "$ICON" 120 "$OUT/App.app/AppIcon60x60@2x.png"
        icon_resize "$ICON" 180 "$OUT/App.app/AppIcon60x60@3x.png"
        icon_resize "$ICON" 152 "$OUT/App.app/AppIcon76x76@2x~ipad.png"
        icon_resize "$ICON" 167 "$OUT/App.app/AppIcon83.5x83.5@2x~ipad.png"
        /usr/libexec/PlistBuddy \
            -c "Add :CFBundleIcons:CFBundlePrimaryIcon:CFBundleIconFiles array" \
            -c "Add :CFBundleIcons:CFBundlePrimaryIcon:CFBundleIconFiles:0 string AppIcon60x60" \
            -c "Add :CFBundleIcons~ipad:CFBundlePrimaryIcon:CFBundleIconFiles array" \
            -c "Add :CFBundleIcons~ipad:CFBundlePrimaryIcon:CFBundleIconFiles:0 string AppIcon60x60" \
            -c "Add :CFBundleIcons~ipad:CFBundlePrimaryIcon:CFBundleIconFiles:1 string AppIcon76x76" \
            -c "Add :CFBundleIcons~ipad:CFBundlePrimaryIcon:CFBundleIconFiles:2 string AppIcon83.5x83.5" \
            "$OUT/App.app/Info.plist"
    fi
fi
if [ -d "$ROOT/mobile/host/fonts" ]; then
    mkdir -p "$OUT/App.app/fonts"
    cp "$ROOT"/mobile/host/fonts/* "$OUT/App.app/fonts/" 2>/dev/null || true
fi
codesign --force --sign - "$OUT/App.app" >/dev/null 2>&1 || true
echo "app: $OUT/App.app ($ID)"
