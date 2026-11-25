#!/bin/sh
# Builds a page or program with the native UI into an Android APK.
#
# Usage:
#   tools/bash/build-android-app.sh APP.salam --out DIR [--abi LIST]
#       [--id APPLICATION_ID] [--name LABEL] [--icon ICON.png] [--icon-bg COLOR]
#       [--splash-color COLOR] [--splash-icon ICON.png]
#       [--version NAME] [--build NUMBER] [--min-sdk N] [--target-sdk N]
#       [--orientation portrait|landscape|auto] [--category NAME] [--config FILE]
#       [--size] [--split] [--no-strip] [--release]
#       [--aab] [--keystore FILE --keystore-pass P --key-alias A --key-pass P]
#       [--fonts DIR] [--assets DIR] [--permission NAME]... [--scheme NAME]...
#       [--link-host HOST]... [--no-apk] [--run]
#
# --icon ICON.png (one square PNG, 1024 pixels is best) becomes the adaptive
# icon (foreground over the --icon-bg colour, default #FFFFFF, plus a
# monochrome layer for themed icons) and the legacy launcher mipmaps.
#
# --splash-color COLOR and --splash-icon ICON.png set the launch screen (the
# Android 12 splash screen, and a window background on older versions). The
# colour defaults to white, or black in the dark theme, and the icon to --icon.
# The host remembers the page background it drew last and paints it behind the
# next launch, so a dark page does not start with a white frame.
#
# --version NAME and --build NUMBER set versionName and versionCode (1.0 and
# 1 by default). --min-sdk (21 or more, the NDK floor) and --target-sdk (35 by
# default) set the API levels. --orientation locks the screen to portrait or
# landscape; auto, the default, follows the sensor. --category (game, social,
# productivity, music, video, photo, news or maps; the other names are for iOS)
# sets android:appCategory.
#
# --config FILE (default: salam.toml next to APP.salam) supplies any of these
# from its [app], [app.mobile] and [app.android] tables with the option names
# written with underscores: id, name, version, build, icon, icon_bg,
# splash_color, splash_icon, min_sdk, target_sdk, orientation, category, abi,
# fonts, assets, permissions, keystore (a path) and key_alias; the plain
# [android] table is read last. Options on the command line win. Passwords
# come only from --keystore-pass and --key-pass, or from SALAM_KEYSTORE_PASS
# and SALAM_KEY_PASS.
#
# --abi takes a comma separated list of arm64-v8a, armeabi-v7a, x86_64 and
# x86, or "all" for the first three (a release build with no --abi builds
# those three; a debug build only arm64-v8a). One APK holds every ABI listed;
# --split writes one APK per ABI instead, DIR/app-ABI.apk, which is what a
# store that serves by device wants.
#
# --dry-run prints the settings the build would use (the command line over
# salam.toml over the defaults) and stops before it needs the Android SDK.
#
# --size builds for size: -Oz, --gc-sections, identical code folding, and the
# symbols stripped. Release and --size builds strip the library by default;
# --no-strip keeps the symbols. The sizes of every library and APK are printed.
#
# --fonts DIR ships the .ttf, .otf and .ttc files of DIR in the APK. They are
# copied to the app files directory at start and found by the font collection.
#
# --assets DIR ships every file under DIR in the APK. They are copied to
# $HOME/assets at start, so DIR/logo.png is written as "asset:logo.png" in
# an image url. SVG is not supported by the native UI.
#
# --permission NAME (camera, microphone, location, notifications) declares a
# permission in the manifest so device.RequestPermission can ask for it. Calls
# that need a permission themselves, such as device.Vibrate, add it alone.
#
# --scheme NAME makes the app open links like NAME://path, and --link-host HOST
# makes it open https://HOST links (Android app links; the site still has to
# serve /.well-known/assetlinks.json). Read the link with device.LaunchURL and
# device.OnOpenURL.
#
# Salam emits C for each ABI, the NDK compiles it with the host shim in
# mobile/host/android into libsalamapp.so, and a prebuilt Gradle project
# packages the library with the Kotlin shell. DIR/app.apk is the result.

set -eu
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
. "$ROOT/mobile/host/common/icons.sh"
. "$ROOT/mobile/host/common/app_config.sh"

: "${SALAM:=salam}"
APP=
OUT=
ABIS=
ID=
NAME=
BUILD=assembleDebug
APK=1
ICON=
ICON_BG=
SPLASH_COLOR=
SPLASH_ICON=
VERSION=
BUILDNUM=
MINSDK=
TARGETSDK=
ORIENT=
CATEGORY=
CONFIG=
SIZE=0
SPLIT=0
STRIP=
FONTS=
ASSETS=
RUN=0
PERMS=
SCHEMES=
LINK_HOSTS=
AAB=0
DRY=0
KS=
KSPASS=
KEYALIAS=
KEYPASS=
while [ $# -gt 0 ]; do
    case $1 in
    --out)
        OUT=$2
        shift 2
        ;;
    --abi)
        ABIS=$2
        shift 2
        ;;
    --id)
        ID=$2
        shift 2
        ;;
    --name)
        NAME=$2
        shift 2
        ;;
    --release)
        BUILD=assembleRelease
        shift
        ;;
    --icon)
        ICON=$2
        shift 2
        ;;
    --icon-bg)
        ICON_BG=$2
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
    --min-sdk)
        MINSDK=$2
        shift 2
        ;;
    --target-sdk)
        TARGETSDK=$2
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
    --dry-run)
        DRY=1
        shift
        ;;
    --size)
        SIZE=1
        shift
        ;;
    --split)
        SPLIT=1
        shift
        ;;
    --no-strip)
        STRIP=0
        shift
        ;;
    --aab)
        AAB=1
        BUILD=bundleRelease
        shift
        ;;
    --keystore)
        KS=$2
        shift 2
        ;;
    --keystore-pass)
        KSPASS=$2
        shift 2
        ;;
    --key-alias)
        KEYALIAS=$2
        shift 2
        ;;
    --key-pass)
        KEYPASS=$2
        shift 2
        ;;
    --fonts)
        FONTS=$2
        shift 2
        ;;
    --assets)
        ASSETS=$2
        shift 2
        ;;
    --scheme)
        SCHEMES="$SCHEMES $2"
        shift 2
        ;;
    --link-host)
        LINK_HOSTS="$LINK_HOSTS $2"
        shift 2
        ;;
    --permission)
        PERMS="$PERMS $2"
        shift 2
        ;;
    --run)
        RUN=1
        shift
        ;;
    --no-apk)
        APK=0
        shift
        ;;
    -*)
        echo "build-android-app: unknown argument '$1'" >&2
        exit 2
        ;;
    *)
        APP=$1
        shift
        ;;
    esac
done
[ -n "$APP" ] && [ -n "$OUT" ] || {
    echo "usage: build-android-app.sh APP.salam --out DIR [--abi LIST] [--id ID] [--name LABEL]" >&2
    exit 2
}
[ -f "$APP" ] || {
    echo "build-android-app: no such file: $APP" >&2
    exit 2
}

if [ -n "$KS" ]; then
    [ -f "$KS" ] || {
        echo "build-android-app: no such keystore: $KS" >&2
        exit 2
    }
    KS=$(cd "$(dirname "$KS")" && pwd)/$(basename "$KS")
fi

if [ -z "$CONFIG" ] && [ -f "$(dirname "$APP")/salam.toml" ]; then
    CONFIG=$(dirname "$APP")/salam.toml
fi
if [ -n "$CONFIG" ]; then
    [ -f "$CONFIG" ] || {
        echo "build-android-app: no such config file: $CONFIG" >&2
        exit 2
    }
    app_cfg_load "$CONFIG" android
    [ -n "$ID" ] || ID=$(app_cfg_get id)
    [ -n "$NAME" ] || NAME=$(app_cfg_get name)
    [ -n "$VERSION" ] || VERSION=$(app_cfg_get version)
    [ -n "$BUILDNUM" ] || BUILDNUM=$(app_cfg_get build)
    [ -n "$ICON" ] || ICON=$(app_cfg_path icon)
    [ -n "$ICON_BG" ] || ICON_BG=$(app_cfg_get icon_bg)
    [ -n "$SPLASH_COLOR" ] || SPLASH_COLOR=$(app_cfg_get splash_color)
    [ -n "$SPLASH_ICON" ] || SPLASH_ICON=$(app_cfg_path splash_icon)
    [ -n "$MINSDK" ] || MINSDK=$(app_cfg_get min_sdk)
    [ -n "$TARGETSDK" ] || TARGETSDK=$(app_cfg_get target_sdk)
    [ -n "$ORIENT" ] || ORIENT=$(app_cfg_get orientation)
    [ -n "$CATEGORY" ] || CATEGORY=$(app_cfg_get category)
    [ -n "$ABIS" ] || ABIS=$(app_cfg_get abi)
    [ -n "$FONTS" ] || FONTS=$(app_cfg_path fonts)
    [ -n "$ASSETS" ] || ASSETS=$(app_cfg_path assets)
    [ -n "$PERMS" ] || PERMS=$(app_cfg_get permissions)
    [ -n "$SCHEMES" ] || SCHEMES=$(app_cfg_get scheme)
    [ -n "$LINK_HOSTS" ] || LINK_HOSTS=$(app_cfg_get link_host)
    [ -n "$KS" ] || KS=$(app_cfg_path keystore)
    [ -n "$KEYALIAS" ] || KEYALIAS=$(app_cfg_get key_alias)
fi
[ -n "$KSPASS" ] || KSPASS=${SALAM_KEYSTORE_PASS:-}
[ -n "$KEYPASS" ] || KEYPASS=${SALAM_KEY_PASS:-}
if [ -n "$KS" ] && [ ! -f "$KS" ]; then
    echo "build-android-app: no such keystore: $KS" >&2
    exit 2
fi
if [ -n "$BUILDNUM" ]; then
    app_check_int --build "$BUILDNUM" build-android-app || exit 2
fi
if [ -n "$MINSDK" ]; then
    app_check_int --min-sdk "$MINSDK" build-android-app || exit 2
    [ "$MINSDK" -ge 21 ] || {
        echo "build-android-app: --min-sdk must be 21 or more" >&2
        exit 2
    }
fi
if [ -n "$TARGETSDK" ]; then
    app_check_int --target-sdk "$TARGETSDK" build-android-app || exit 2
fi
app_check_orientation "$ORIENT" build-android-app || exit 2
app_category_known "$CATEGORY" build-android-app || exit 2
[ -n "$ICON_BG" ] || ICON_BG="#FFFFFF"
[ "$ABIS" != all ] || ABIS=arm64-v8a,armeabi-v7a,x86_64
if [ -z "$STRIP" ]; then
    STRIP=0
    if [ "$BUILD" != assembleDebug ] || [ "$SIZE" = 1 ]; then
        STRIP=1
    fi
fi
OPT=-O2
LDOPT=
if [ "$SIZE" = 1 ]; then
    OPT=-Oz
    LDOPT="-Wl,--icf=all"
fi
if [ "$STRIP" = 1 ]; then
    LDOPT="$LDOPT -Wl,--strip-all"
fi
if [ -z "$ABIS" ]; then
    if [ "$BUILD" = assembleDebug ]; then
        ABIS=arm64-v8a
    else
        ABIS=arm64-v8a,armeabi-v7a,x86_64
    fi
fi

if [ "$DRY" = 1 ]; then
    for kv in "id=$ID" "name=$NAME" "version=$VERSION" "build=$BUILDNUM" "icon=$ICON" "icon_bg=$ICON_BG" \
        "splash_color=$SPLASH_COLOR" "splash_icon=$SPLASH_ICON" "min_sdk=$MINSDK" "target_sdk=$TARGETSDK" \
        "orientation=$ORIENT" "category=$CATEGORY" "abi=$ABIS" "fonts=$FONTS" "assets=$ASSETS" \
        "permissions=$PERMS" "keystore=$KS" "key_alias=$KEYALIAS" "keystore_pass=${KSPASS:+set}" \
        "key_pass=${KEYPASS:+set}"; do
        printf '%s\n' "$kv"
    done
    exit 0
fi

SDK=${ANDROID_HOME:-${ANDROID_SDK_ROOT:-$HOME/Library/Android/sdk}}
[ -d "$SDK" ] || SDK=$HOME/Android/Sdk
[ -d "$SDK" ] || {
    echo "build-android-app: no Android SDK; install it and set ANDROID_HOME to its folder" >&2
    exit 1
}
NDK=${ANDROID_NDK_HOME:-}
if [ -z "$NDK" ]; then
    NDK=$(find "$SDK/ndk" -mindepth 1 -maxdepth 1 -type d 2>/dev/null | sort | tail -n 1)
fi
[ -d "$NDK" ] || {
    echo "build-android-app: no Android NDK in $SDK/ndk; install one (sdkmanager \"ndk;28.2.13676358\") or set ANDROID_NDK_HOME" >&2
    exit 1
}
if [ "$APK" = 1 ] && [ -z "${JAVA_HOME:-}" ] && ! java -version >/dev/null 2>&1; then
    echo "build-android-app: no JDK; install JDK 21 and set JAVA_HOME to it" >&2
    exit 1
fi
if [ "$APK" = 1 ] && [ -n "${JAVA_HOME:-}" ] && [ ! -x "$JAVA_HOME/bin/java" ]; then
    echo "build-android-app: JAVA_HOME ($JAVA_HOME) has no bin/java; point it at a JDK 21" >&2
    exit 1
fi
case $(uname -s) in
Darwin) HOST=darwin-x86_64 ;;
*) HOST=linux-x86_64 ;;
esac
BIN=$NDK/toolchains/llvm/prebuilt/$HOST/bin

APP=$(cd "$(dirname "$APP")" && pwd)/$(basename "$APP")
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)
base=$(basename "$APP" .salam)
[ -n "$ID" ] || ID=app.salam.$(printf %s "$base" | tr -c 'A-Za-z0-9\n' '_' | tr '[:upper:]' '[:lower:]')
[ -n "$NAME" ] || NAME=$base

SQLITE_VERSION=3500400
SQLITE_SHA256=1d3049dd0f830a025a53105fc79fd2ab9431aea99e137809d064d8ee8356b032

fetch_sqlite() {
    cache=${XDG_CACHE_HOME:-$HOME/.cache}/salam/sqlite-$SQLITE_VERSION
    if [ ! -f "$cache/sqlite3.c" ]; then
        mkdir -p "$cache"
        zip=$cache/sqlite.zip
        curl -fsSL -o "$zip" "https://www.sqlite.org/2025/sqlite-amalgamation-$SQLITE_VERSION.zip"
        got=$(shasum -a 256 "$zip" | cut -d' ' -f1)
        [ "$got" = "$SQLITE_SHA256" ] || {
            echo "build-android-app: sqlite download does not match its checksum" >&2
            rm -f "$zip"
            exit 1
        }
        unzip -q -o -j "$zip" "sqlite-amalgamation-$SQLITE_VERSION/sqlite3.c" "sqlite-amalgamation-$SQLITE_VERSION/sqlite3.h" -d "$cache"
        rm -f "$zip"
    fi
    echo "$cache"
}

rm -rf "$OUT/project"
mkdir -p "$OUT/project"
cp -R "$ROOT/mobile/host/android/project/." "$OUT/project/"
chmod -R u+w "$OUT/project"
if [ -n "$ICON" ]; then
    icon_check "$ICON" build-android-app || exit 2
    RES=$OUT/project/app/src/main/res
    mkdir -p "$RES/drawable" "$RES/drawable-nodpi" "$RES/mipmap-anydpi-v26" "$RES/values"
    icon_resize "$ICON" 432 "$RES/drawable-nodpi/ic_launcher_art.png"
    for pair in mdpi:48 hdpi:72 xhdpi:96 xxhdpi:144 xxxhdpi:192; do
        mkdir -p "$RES/mipmap-${pair%%:*}"
        icon_resize "$ICON" "${pair##*:}" "$RES/mipmap-${pair%%:*}/ic_launcher.png"
    done
    cat >"$RES/drawable/ic_launcher_foreground.xml" <<XML
<?xml version="1.0" encoding="utf-8"?>
<inset xmlns:android="http://schemas.android.com/apk/res/android"
    android:drawable="@drawable/ic_launcher_art"
    android:inset="16.667%" />
XML
    cat >"$RES/values/ic_launcher_background.xml" <<XML
<?xml version="1.0" encoding="utf-8"?>
<resources>
    <color name="ic_launcher_background">$ICON_BG</color>
</resources>
XML
    cat >"$RES/mipmap-anydpi-v26/ic_launcher.xml" <<XML
<?xml version="1.0" encoding="utf-8"?>
<adaptive-icon xmlns:android="http://schemas.android.com/apk/res/android">
    <background android:drawable="@color/ic_launcher_background" />
    <foreground android:drawable="@drawable/ic_launcher_foreground" />
    <monochrome android:drawable="@drawable/ic_launcher_foreground" />
</adaptive-icon>
XML
    sed -i.bak 's|android:supportsRtl="true"|android:supportsRtl="true" android:icon="@mipmap/ic_launcher" android:roundIcon="@mipmap/ic_launcher"|' \
        "$OUT/project/app/src/main/AndroidManifest.xml"
    rm -f "$OUT/project/app/src/main/AndroidManifest.xml.bak"
fi
if [ -n "$SPLASH_COLOR" ]; then
    printf %s "$SPLASH_COLOR" | grep -Eq '^#([0-9A-Fa-f]{3}|[0-9A-Fa-f]{6}|[0-9A-Fa-f]{8})$' || {
        echo "build-android-app: --splash-color wants #RGB, #RRGGBB or #AARRGGBB, not '$SPLASH_COLOR'" >&2
        exit 2
    }
    for dir in values values-night; do
        mkdir -p "$OUT/project/app/src/main/res/$dir"
        cat >"$OUT/project/app/src/main/res/$dir/splash.xml" <<XML
<?xml version="1.0" encoding="utf-8"?>
<resources>
    <color name="splash_background">$SPLASH_COLOR</color>
</resources>
XML
    done
fi
[ -z "$SPLASH_ICON" ] && SPLASH_ICON=$ICON
if [ -n "$SPLASH_ICON" ]; then
    icon_check "$SPLASH_ICON" build-android-app || exit 2
    RES=$OUT/project/app/src/main/res
    mkdir -p "$RES/drawable-nodpi"
    icon_resize "$SPLASH_ICON" 288 "$RES/drawable-nodpi/splash_art.png"
    cat >"$RES/drawable/splash_icon.xml" <<XML
<?xml version="1.0" encoding="utf-8"?>
<inset xmlns:android="http://schemas.android.com/apk/res/android"
    android:drawable="@drawable/splash_art"
    android:inset="25%" />
XML
    cat >"$RES/drawable/splash_window.xml" <<XML
<?xml version="1.0" encoding="utf-8"?>
<layer-list xmlns:android="http://schemas.android.com/apk/res/android">
    <item android:drawable="@color/splash_background" />
    <item>
        <bitmap android:src="@drawable/splash_art" android:gravity="center" />
    </item>
</layer-list>
XML
    for sf in values values-night; do
        sed -i.bak 's|</style>|        <item name="android:windowSplashScreenAnimatedIcon">@drawable/splash_icon</item>\
    </style>|' "$RES/$sf/styles.xml"
        rm -f "$RES/$sf/styles.xml.bak"
    done
fi
if [ -n "$FONTS" ]; then
    [ -d "$FONTS" ] || {
        echo "build-android-app: no such fonts directory: $FONTS" >&2
        exit 2
    }
    mkdir -p "$OUT/project/app/src/main/assets/fonts"
    for f in "$FONTS"/*.ttf "$FONTS"/*.otf "$FONTS"/*.ttc; do
        if [ -f "$f" ]; then
            cp "$f" "$OUT/project/app/src/main/assets/fonts/"
        fi
    done
fi
if [ -n "$ASSETS" ]; then
    [ -d "$ASSETS" ] || {
        echo "build-android-app: no such assets directory: $ASSETS" >&2
        exit 2
    }
    mkdir -p "$OUT/project/app/src/main/assets/assets"
    cp -R "$ASSETS"/. "$OUT/project/app/src/main/assets/assets/"
fi
mkdir -p "$OUT/project/gradle/wrapper"
cp "$ROOT/android/gradle/wrapper/gradle-wrapper.jar" "$ROOT/android/gradle/wrapper/gradle-wrapper.properties" "$OUT/project/gradle/wrapper/"
cp "$ROOT/android/gradlew" "$OUT/project/gradlew"
chmod +x "$OUT/project/gradlew"

old_ifs=$IFS
IFS=,
for abi in $ABIS; do
    IFS=$old_ifs
    case $abi in
    arm64-v8a) triple=aarch64-linux-android clang=aarch64-linux-android21 ;;
    armeabi-v7a) triple=armv7a-linux-androideabi clang=armv7a-linux-androideabi21 ;;
    x86_64) triple=x86_64-linux-android clang=x86_64-linux-android21 ;;
    x86) triple=i686-linux-android clang=i686-linux-android21 ;;
    *)
        echo "build-android-app: unknown ABI '$abi'" >&2
        exit 2
        ;;
    esac
    work="$OUT/work/$abi"
    rm -rf "$work"
    mkdir -p "$work/obj"
    (
        cd "$work"
        "$SALAM" build "$APP" --ui=native --backend=c --emit-c --target="$triple" \
            --output="$work/app" --log-level=warn
    )
    CC=$BIN/$clang-clang
    for f in "$work"/.salam-build/*.c; do
        "$CC" $OPT -w -fPIC -fno-strict-aliasing -ffunction-sections -fdata-sections \
            -Dmain=salam_app_main -I"$work/.salam-build" -c "$f" -o "$work/obj/$(basename "$f").o"
    done
    if grep -q 'sqlite3_open' "$work"/.salam-build/*.c 2>/dev/null; then
        sqlite_dir=$(fetch_sqlite)
        "$CC" $OPT -fPIC -w -DSQLITE_THREADSAFE=1 -DSQLITE_OMIT_LOAD_EXTENSION -c \
            "$sqlite_dir/sqlite3.c" -o "$work/obj/sqlite3.o"
    fi
    "$CC" $OPT -fPIC -c "$ROOT/mobile/host/android/salam_host.c" -o "$work/obj/salam_host.o"
    "$CC" $OPT -fPIC -c "$ROOT/mobile/host/common/salam_host_common.c" -o "$work/obj/salam_host_common.o"
    lib="$OUT/project/app/src/main/jniLibs/$abi"
    mkdir -p "$lib"
    "$CC" -shared -o "$lib/libsalamapp.so" "$work"/obj/*.o -landroid -llog -lm -ldl \
        -Wl,--gc-sections -Wl,-z,max-page-size=16384 $LDOPT
    echo "built $lib/libsalamapp.so ($(wc -c <"$lib/libsalamapp.so" | tr -d ' ') bytes)"
    IFS=,
done
IFS=$old_ifs

MANIFEST=$OUT/project/app/src/main/AndroidManifest.xml
case $ORIENT in
portrait | landscape)
    sed -i.bak "s|android:launchMode=\"singleTask\"|android:launchMode=\"singleTask\" android:screenOrientation=\"$ORIENT\"|" "$MANIFEST"
    rm -f "$MANIFEST.bak"
    ;;
esac
case $CATEGORY in
game | social | productivity | news | maps | video) APP_CATEGORY=$CATEGORY ;;
music) APP_CATEGORY=audio ;;
photo) APP_CATEGORY=image ;;
*) APP_CATEGORY= ;;
esac
if [ -n "$APP_CATEGORY" ]; then
    sed -i.bak "s|android:label=\"@string/app_name\"|android:label=\"@string/app_name\" android:appCategory=\"$APP_CATEGORY\"|" "$MANIFEST"
    rm -f "$MANIFEST.bak"
fi
FILTERS=
VIEW='<action android:name="android.intent.action.VIEW" />'
VIEW="$VIEW"'<category android:name="android.intent.category.DEFAULT" />'
VIEW="$VIEW"'<category android:name="android.intent.category.BROWSABLE" />'
for name in $SCHEMES; do
    FILTERS="$FILTERS<intent-filter>$VIEW<data android:scheme=\"$name\" /></intent-filter>"
done
for name in $LINK_HOSTS; do
    FILTERS="$FILTERS<intent-filter android:autoVerify=\"true\">$VIEW"
    FILTERS="$FILTERS<data android:scheme=\"https\" android:host=\"$name\" /></intent-filter>"
done
if [ -n "$FILTERS" ]; then
    awk -v add="$FILTERS" '/<\/activity>/ { print add } { print }' "$MANIFEST" >"$MANIFEST.new"
    mv "$MANIFEST.new" "$MANIFEST"
fi
declare_permission() {
    grep -q "permission.$1\"" "$MANIFEST" && return 0
    awk -v line="    <uses-permission android:name=\"android.permission.$1\" />" \
        '{ print } /android.permission.INTERNET/ { print line }' "$MANIFEST" >"$MANIFEST.new"
    mv "$MANIFEST.new" "$MANIFEST"
}
uses_host_call() {
    grep -qsw "$1" "$OUT"/work/*/.salam-build/*.c
}
if uses_host_call salam_host_vibrate; then
    declare_permission VIBRATE
fi
if uses_host_call salam_host_location_request; then
    declare_permission ACCESS_FINE_LOCATION
    declare_permission ACCESS_COARSE_LOCATION
fi
if uses_host_call salam_host_auth_request; then
    declare_permission USE_BIOMETRIC
fi
if uses_host_call salam_host_network; then
    declare_permission ACCESS_NETWORK_STATE
fi
if uses_host_call salam_host_notify; then
    declare_permission POST_NOTIFICATIONS
fi
for name in $PERMS; do
    case $name in
    camera) declare_permission CAMERA ;;
    microphone) declare_permission RECORD_AUDIO ;;
    location)
        declare_permission ACCESS_FINE_LOCATION
        declare_permission ACCESS_COARSE_LOCATION
        ;;
    notifications) declare_permission POST_NOTIFICATIONS ;;
    *)
        echo "build-android-app: unknown permission '$name' (camera, microphone, location, notifications)" >&2
        exit 2
        ;;
    esac
done

[ "$APK" = 1 ] || exit 0
export ANDROID_HOME=$SDK
set -- "$BUILD" "-PappId=$ID" "-PappName=$NAME"
[ "$SPLIT" = 0 ] || set -- "$@" "-PsplitAbis=$ABIS"
[ -z "$VERSION" ] || set -- "$@" "-PappVersion=$VERSION"
[ -z "$BUILDNUM" ] || set -- "$@" "-PappVersionCode=$BUILDNUM"
[ -z "$MINSDK" ] || set -- "$@" "-PminSdkVersion=$MINSDK"
[ -z "$TARGETSDK" ] || set -- "$@" "-PtargetSdkVersion=$TARGETSDK"
if [ -n "$KS" ]; then
    set -- "$@" "-PstoreFile=$KS" "-PstorePassword=$KSPASS" "-PkeyAlias=$KEYALIAS" "-PkeyPassword=${KEYPASS:-$KSPASS}"
fi
(
    cd "$OUT/project"
    ./gradlew --no-daemon -q "$@"
)
if [ "$AAB" = 1 ]; then
    aab=$(find "$OUT/project/app/build/outputs/bundle" -name '*.aab' | head -n 1)
    [ -n "$aab" ] || {
        echo "build-android-app: Gradle produced no AAB" >&2
        exit 1
    }
    cp "$aab" "$OUT/app.aab"
    echo "aab: $OUT/app.aab"
    exit 0
fi
if [ "$SPLIT" = 1 ]; then
    rm -f "$OUT"/app-*.apk
    for f in "$OUT"/project/app/build/outputs/apk/*/*.apk; do
        n=$(basename "$f" .apk)
        n=${n#app-}
        n=${n%-unsigned}
        n=${n%-debug}
        n=${n%-release}
        cp "$f" "$OUT/app-$n.apk"
        echo "apk: $OUT/app-$n.apk ($(wc -c <"$OUT/app-$n.apk" | tr -d ' ') bytes)"
    done
    exit 0
fi
apk=$(find "$OUT/project/app/build/outputs/apk" -name '*.apk' | head -n 1)
[ -n "$apk" ] || {
    echo "build-android-app: Gradle produced no APK" >&2
    exit 1
}
cp "$apk" "$OUT/app.apk"
echo "apk: $OUT/app.apk ($(wc -c <"$OUT/app.apk" | tr -d ' ') bytes)"
if [ "$RUN" = 1 ]; then
    ADB=$SDK/platform-tools/adb
    "$ADB" install -r "$OUT/app.apk"
    "$ADB" shell am start -n "$ID/ir.salamlang.host.SalamActivity"
    echo "logs: $ADB logcat -s salam"
fi
