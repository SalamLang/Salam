#!/bin/sh
# Builds a page or program with the native UI into an iOS app bundle.
#
# Usage:
#   tools/bash/build-ios-app.sh APP.salam --out DIR [--device] [--id BUNDLE_ID] [--name LABEL]
#       [--icon ICON.png] [--splash-color COLOR] [--splash-icon ICON.png]
#       [--version NAME] [--build NUMBER] [--min-ios VERSION]
#       [--orientation portrait|landscape|auto] [--category NAME] [--config FILE]
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
# --splash-color COLOR (#RGB, #RRGGBB or #AARRGGBB) and --splash-icon ICON.png
# fill the UILaunchScreen dictionary through the same asset catalog. Without
# them the launch screen is the system background, white or black to match the
# appearance. The host remembers the page background it drew last and uses it
# behind the next launch, so a dark page does not start with a white frame.
#
# --version NAME and --build NUMBER set CFBundleShortVersionString and
# CFBundleVersion (1.0 and 1 by default). --min-ios VERSION (15.0 by default)
# is both MinimumOSVersion and the deployment target of the compile.
# --orientation portrait or landscape limits the supported orientations; auto,
# the default, allows portrait and both landscapes. --category (game, social,
# productivity, utilities, education, finance, health, lifestyle, music, news,
# photo, travel, weather, business, entertainment, sports, books, food, maps,
# video or reference) sets LSApplicationCategoryType.
#
# --config FILE (default: salam.toml next to APP.salam) supplies any of these
# from its [app], [app.mobile] and [app.ios] tables with the option names
# written with underscores: id, name, version, build, icon, splash_color,
# splash_icon, min_ios, orientation, category and permissions. Options on the
# command line win.
#
# --permission NAME (camera, microphone, location, notifications) adds the
# usage description the system shows in its permission dialog, so
# device.RequestPermission can ask for it. Calls that need a description
# themselves, such as device.TakePhoto or device.RequestLocation, add it alone.

set -eu
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
. "$ROOT/mobile/host/common/icons.sh"
. "$ROOT/mobile/host/common/app_config.sh"

: "${SALAM:=salam}"
APP=
OUT=
DEVICE=0
ID=
NAME=
PERMS=
ICON=
SPLASH_COLOR=
SPLASH_ICON=
VERSION=
BUILDNUM=
MINIOS=
ORIENT=
CATEGORY=
CONFIG=
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
    --splash-color)
        SPLASH_COLOR=$2
        shift 2
        ;;
    --splash-icon)
        SPLASH_ICON=$2
        shift 2
        ;;
    --version)
        VERSION=$2
        shift 2
        ;;
    --build)
        BUILDNUM=$2
        shift 2
        ;;
    --min-ios)
        MINIOS=$2
        shift 2
        ;;
    --orientation)
        ORIENT=$2
        shift 2
        ;;
    --category)
        CATEGORY=$2
        shift 2
        ;;
    --config)
        CONFIG=$2
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
if [ -z "$CONFIG" ] && [ -f "$(dirname "$APP")/salam.toml" ]; then
    CONFIG=$(dirname "$APP")/salam.toml
fi
if [ -n "$CONFIG" ]; then
    [ -f "$CONFIG" ] || {
        echo "build-ios-app: no such config file: $CONFIG" >&2
        exit 2
    }
    app_cfg_load "$CONFIG" ios
    [ -n "$ID" ] || ID=$(app_cfg_get id)
    [ -n "$NAME" ] || NAME=$(app_cfg_get name)
    [ -n "$VERSION" ] || VERSION=$(app_cfg_get version)
    [ -n "$BUILDNUM" ] || BUILDNUM=$(app_cfg_get build)
    [ -n "$ICON" ] || ICON=$(app_cfg_path icon)
    [ -n "$SPLASH_COLOR" ] || SPLASH_COLOR=$(app_cfg_get splash_color)
    [ -n "$SPLASH_ICON" ] || SPLASH_ICON=$(app_cfg_path splash_icon)
    [ -n "$MINIOS" ] || MINIOS=$(app_cfg_get min_ios)
    [ -n "$ORIENT" ] || ORIENT=$(app_cfg_get orientation)
    [ -n "$CATEGORY" ] || CATEGORY=$(app_cfg_get category)
    [ -n "$PERMS" ] || PERMS=$(app_cfg_get permissions)
fi
if [ -n "$BUILDNUM" ]; then
    app_check_int --build "$BUILDNUM" build-ios-app || exit 2
fi
case $MINIOS in
"" | [0-9] | [0-9].[0-9] | [0-9][0-9] | [0-9][0-9].[0-9] | [0-9][0-9].[0-9][0-9] | [0-9].[0-9][0-9]) ;;
*)
    echo "build-ios-app: --min-ios wants a version such as 15.0, not '$MINIOS'" >&2
    exit 2
    ;;
esac
[ -n "$MINIOS" ] || MINIOS=15.0
app_check_orientation "$ORIENT" build-ios-app || exit 2
app_category_known "$CATEGORY" build-ios-app || exit 2
if [ -n "$ICON" ]; then
    icon_check "$ICON" build-ios-app || exit 2
    ICON=$(cd "$(dirname "$ICON")" && pwd)/$(basename "$ICON")
fi
if [ -n "$SPLASH_ICON" ]; then
    icon_check "$SPLASH_ICON" build-ios-app || exit 2
    SPLASH_ICON=$(cd "$(dirname "$SPLASH_ICON")" && pwd)/$(basename "$SPLASH_ICON")
fi
LAUNCH=
if [ -n "$SPLASH_COLOR" ]; then
    hex=${SPLASH_COLOR#"#"}
    case $hex in
    [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]) hex=$(printf %s "$hex" | sed 's/./&&/g') ;;
    esac
    case $hex in
    [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]) hex=FF$hex ;;
    [0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]) ;;
    *)
        echo "build-ios-app: --splash-color wants #RGB, #RRGGBB or #AARRGGBB, not '$SPLASH_COLOR'" >&2
        exit 2
        ;;
    esac
    SPLASH_COMPONENTS=$(printf %s "$hex" | awk '{
        h[1] = substr($0, 1, 2); h[2] = substr($0, 3, 2); h[3] = substr($0, 5, 2); h[4] = substr($0, 7, 2)
        for (i = 1; i <= 4; i++) {
            v = 0
            for (j = 1; j <= 2; j++) {
                v = v * 16 + index("0123456789ABCDEF", toupper(substr(h[i], j, 1))) - 1
            }
            out[i] = v / 255
        }
        printf "\"red\":\"%.4f\",\"green\":\"%.4f\",\"blue\":\"%.4f\",\"alpha\":\"%.4f\"", out[2], out[3], out[4], out[1]
    }')
    LAUNCH="<key>UIColorName</key><string>SplashColor</string>"
fi
if [ -n "$SPLASH_ICON" ]; then
    LAUNCH="$LAUNCH<key>UIImageName</key><string>SplashIcon</string>"
fi

APP=$(cd "$(dirname "$APP")" && pwd)/$(basename "$APP")
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)
base=$(basename "$APP" .salam)
[ -n "$ID" ] || ID=app.salam.$(printf %s "$base" | tr -c 'A-Za-z0-9\n' '_' | tr '[:upper:]' '[:lower:]')
[ -n "$NAME" ] || NAME=$base

if [ "$DEVICE" = 1 ]; then
    SDK=iphoneos
    TRIPLE=arm64-apple-ios$MINIOS
else
    SDK=iphonesimulator
    TRIPLE=arm64-apple-ios$MINIOS-simulator
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
uses_host_call() {
    grep -qsw "$1" "$work"/.salam-build/*.c
}
SIM_ENT=
if [ "$DEVICE" = 0 ] && uses_host_call salam_host_secure_set; then
    cat >"$work/entitlements.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
<key>application-identifier</key><string>$ID</string>
<key>keychain-access-groups</key><array><string>$ID</string></array>
</dict></plist>
PLIST
    SIM_ENT="-Wl,-sectcreate,__TEXT,__entitlements,$work/entitlements.plist"
fi
$CC -o "$OUT/App.app/App" "$work"/obj/*.o -lm \
    -framework UIKit -framework Foundation -framework CoreGraphics -framework QuartzCore \
    -framework AVFoundation -framework AudioToolbox -framework CoreLocation -framework PhotosUI \
    -framework UserNotifications -framework Security $SIM_ENT

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
add_category() {
    USAGE_KEYS="$USAGE_KEYS
    <key>LSApplicationCategoryType</key><string>public.app-category.$1</string>"
}
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

case $ORIENT in
portrait) ORIENT_KEYS='<string>UIInterfaceOrientationPortrait</string>' ;;
landscape)
    ORIENT_KEYS='<string>UIInterfaceOrientationLandscapeLeft</string>
        <string>UIInterfaceOrientationLandscapeRight</string>'
    ;;
*)
    ORIENT_KEYS='<string>UIInterfaceOrientationPortrait</string>
        <string>UIInterfaceOrientationLandscapeLeft</string>
        <string>UIInterfaceOrientationLandscapeRight</string>'
    ;;
esac
case $CATEGORY in
"") ;;
game) add_category games ;;
social) add_category social-networking ;;
health) add_category healthcare-fitness ;;
photo) add_category photography ;;
food) add_category food-drink ;;
maps) add_category navigation ;;
*) add_category "$CATEGORY" ;;
esac

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
    <key>CFBundleShortVersionString</key><string>${VERSION:-1.0}</string>
    <key>CFBundleVersion</key><string>${BUILDNUM:-1}</string>
    <key>MinimumOSVersion</key><string>$MINIOS</string>
    <key>UILaunchScreen</key><dict>$LAUNCH</dict>$USAGE_KEYS
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
        $ORIENT_KEYS
    </array>
</dict>
</plist>
PLIST
if [ -n "$ICON" ] || [ -n "$SPLASH_COLOR" ] || [ -n "$SPLASH_ICON" ]; then
    cat_dir=$work/icon/Assets.xcassets
    mkdir -p "$cat_dir"
    printf '%s\n' '{"info":{"author":"xcode","version":1}}' >"$cat_dir/Contents.json"
    ACTOOL_ICON=
    if [ -n "$ICON" ]; then
        ACTOOL_ICON="--app-icon AppIcon"
        mkdir -p "$cat_dir/AppIcon.appiconset"
        cp "$ICON" "$cat_dir/AppIcon.appiconset/icon.png"
        printf '%s\n' '{"images":[{"filename":"icon.png","idiom":"universal","platform":"ios","size":"1024x1024"}],"info":{"author":"xcode","version":1}}' \
            >"$cat_dir/AppIcon.appiconset/Contents.json"
    fi
    if [ -n "$SPLASH_COLOR" ]; then
        mkdir -p "$cat_dir/SplashColor.colorset"
        printf '%s\n' "{\"colors\":[{\"color\":{\"color-space\":\"srgb\",\"components\":{$SPLASH_COMPONENTS}},\"idiom\":\"universal\"}],\"info\":{\"author\":\"xcode\",\"version\":1}}" \
            >"$cat_dir/SplashColor.colorset/Contents.json"
    fi
    if [ -n "$SPLASH_ICON" ]; then
        mkdir -p "$cat_dir/SplashIcon.imageset"
        icon_resize "$SPLASH_ICON" 360 "$cat_dir/SplashIcon.imageset/splash.png"
        printf '%s\n' '{"images":[{"filename":"splash.png","idiom":"universal","scale":"3x"}],"info":{"author":"xcode","version":1}}' \
            >"$cat_dir/SplashIcon.imageset/Contents.json"
    fi
    if xcrun actool --compile "$OUT/App.app" --platform "$SDK" --minimum-deployment-target "$MINIOS" \
        $ACTOOL_ICON --target-device iphone --target-device ipad \
        --output-partial-info-plist "$work/icon/partial.plist" "$cat_dir" >"$work/icon/actool.log" 2>&1 &&
        [ -f "$work/icon/partial.plist" ]; then
        if [ -n "$ICON" ]; then
            /usr/libexec/PlistBuddy -c "Merge $work/icon/partial.plist" "$OUT/App.app/Info.plist"
        fi
    elif [ -n "$ICON" ]; then
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
    else
        echo "build-ios-app: actool failed, the launch screen has no colour or image" >&2
        /usr/libexec/PlistBuddy -c "Delete :UILaunchScreen" -c "Add :UILaunchScreen dict" "$OUT/App.app/Info.plist"
    fi
fi
if [ -d "$ROOT/mobile/host/fonts" ]; then
    mkdir -p "$OUT/App.app/fonts"
    cp "$ROOT"/mobile/host/fonts/* "$OUT/App.app/fonts/" 2>/dev/null || true
fi
codesign --force --sign - "$OUT/App.app" >/dev/null 2>&1 || true
echo "app: $OUT/App.app ($ID)"
