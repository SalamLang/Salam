#!/bin/sh
# Builds a page or program with the native UI into an Android APK.
#
# Usage:
#   tools/bash/build-android-app.sh APP.salam --out DIR [--abi LIST]
#       [--id APPLICATION_ID] [--name LABEL] [--icon ICON.png] [--release]
#       [--aab] [--keystore FILE --keystore-pass P --key-alias A --key-pass P]
#       [--fonts DIR] [--permission NAME]... [--no-apk] [--run]
#
# --fonts DIR ships the .ttf, .otf and .ttc files of DIR in the APK. They are
# copied to the app files directory at start and found by the font collection.
#
# --permission NAME (camera, microphone, location, notifications) declares a
# permission in the manifest so device.RequestPermission can ask for it. Calls
# that need a permission themselves, such as device.Vibrate, add it alone.
#
# Salam emits C for each ABI, the NDK compiles it with the host shim in
# mobile/host/android into libsalamapp.so, and a prebuilt Gradle project
# packages the library with the Kotlin shell. DIR/app.apk is the result.

set -eu
ROOT=$(cd "$(dirname "$0")/../.." && pwd)

: "${SALAM:=salam}"
APP=
OUT=
ABIS=
ID=
NAME=
BUILD=assembleDebug
APK=1
ICON=
FONTS=
RUN=0
PERMS=
AAB=0
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

if [ -z "$ABIS" ]; then
    if [ "$BUILD" = assembleDebug ]; then
        ABIS=arm64-v8a
    else
        ABIS=arm64-v8a,armeabi-v7a,x86_64
    fi
fi

SDK=${ANDROID_HOME:-${ANDROID_SDK_ROOT:-$HOME/Library/Android/sdk}}
[ -d "$SDK" ] || SDK=$HOME/Android/Sdk
NDK=${ANDROID_NDK_HOME:-}
if [ -z "$NDK" ]; then
    NDK=$(find "$SDK/ndk" -mindepth 1 -maxdepth 1 -type d 2>/dev/null | sort | tail -n 1)
fi
[ -d "$NDK" ] || {
    echo "build-android-app: no Android NDK; set ANDROID_NDK_HOME" >&2
    exit 1
}
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
if [ -n "$ICON" ]; then
    [ -f "$ICON" ] || {
        echo "build-android-app: no such icon: $ICON" >&2
        exit 2
    }
    mkdir -p "$OUT/project/app/src/main/res/mipmap-xxxhdpi"
    cp "$ICON" "$OUT/project/app/src/main/res/mipmap-xxxhdpi/ic_launcher.png"
    sed -i.bak 's|android:supportsRtl="true"|android:supportsRtl="true" android:icon="@mipmap/ic_launcher"|' \
        "$OUT/project/app/src/main/AndroidManifest.xml"
    rm -f "$OUT/project/app/src/main/AndroidManifest.xml.bak"
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
        "$CC" -O2 -w -fPIC -fno-strict-aliasing -ffunction-sections -fdata-sections \
            -Dmain=salam_app_main -I"$work/.salam-build" -c "$f" -o "$work/obj/$(basename "$f").o"
    done
    if grep -q 'sqlite3_open' "$work"/.salam-build/*.c 2>/dev/null; then
        sqlite_dir=$(fetch_sqlite)
        "$CC" -O2 -fPIC -w -DSQLITE_THREADSAFE=1 -DSQLITE_OMIT_LOAD_EXTENSION -c \
            "$sqlite_dir/sqlite3.c" -o "$work/obj/sqlite3.o"
    fi
    "$CC" -O2 -fPIC -c "$ROOT/mobile/host/android/salam_host.c" -o "$work/obj/salam_host.o"
    "$CC" -O2 -fPIC -c "$ROOT/mobile/host/common/salam_host_common.c" -o "$work/obj/salam_host_common.o"
    lib="$OUT/project/app/src/main/jniLibs/$abi"
    mkdir -p "$lib"
    "$CC" -shared -o "$lib/libsalamapp.so" "$work"/obj/*.o -landroid -llog -lm -ldl \
        -Wl,--gc-sections -Wl,-z,max-page-size=16384
    echo "built $lib/libsalamapp.so"
    IFS=,
done
IFS=$old_ifs

MANIFEST=$OUT/project/app/src/main/AndroidManifest.xml
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
SIGN=
if [ -n "$KS" ]; then
    SIGN="-PstoreFile=$KS -PstorePassword=$KSPASS -PkeyAlias=$KEYALIAS -PkeyPassword=${KEYPASS:-$KSPASS}"
fi
(
    cd "$OUT/project"
    # shellcheck disable=SC2086
    ./gradlew --no-daemon -q "$BUILD" -PappId="$ID" -PappName="$NAME" $SIGN
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
apk=$(find "$OUT/project/app/build/outputs/apk" -name '*.apk' | head -n 1)
[ -n "$apk" ] || {
    echo "build-android-app: Gradle produced no APK" >&2
    exit 1
}
cp "$apk" "$OUT/app.apk"
echo "apk: $OUT/app.apk"
if [ "$RUN" = 1 ]; then
    ADB=$SDK/platform-tools/adb
    "$ADB" install -r "$OUT/app.apk"
    "$ADB" shell am start -n "$ID/ir.salamlang.host.SalamActivity"
    echo "logs: $ADB logcat -s salam"
fi
