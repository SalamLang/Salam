#!/bin/sh
# Builds a page or program with the native UI into an Android APK.
#
# Usage:
#   tools/bash/build-android-app.sh APP.salam --out DIR [--abi LIST]
#       [--id APPLICATION_ID] [--name LABEL] [--release] [--no-apk]
#
# Salam emits C for each ABI, the NDK compiles it with the host shim in
# mobile/host/android into libsalamapp.so, and a prebuilt Gradle project
# packages the library with the Kotlin shell. DIR/app.apk is the result.

set -eu
ROOT=$(cd "$(dirname "$0")/../.." && pwd)

: "${SALAM:=salam}"
APP=
OUT=
ABIS=arm64-v8a
ID=
NAME=
BUILD=assembleDebug
APK=1
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

rm -rf "$OUT/project"
mkdir -p "$OUT/project"
cp -R "$ROOT/mobile/host/android/project/." "$OUT/project/"
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
    "$CC" -O2 -fPIC -c "$ROOT/mobile/host/android/salam_host.c" -o "$work/obj/salam_host.o"
    lib="$OUT/project/app/src/main/jniLibs/$abi"
    mkdir -p "$lib"
    "$CC" -shared -o "$lib/libsalamapp.so" "$work"/obj/*.o -landroid -llog -lm -ldl \
        -Wl,--gc-sections -Wl,-z,max-page-size=16384
    echo "built $lib/libsalamapp.so"
    IFS=,
done
IFS=$old_ifs

[ "$APK" = 1 ] || exit 0
export ANDROID_HOME=$SDK
(
    cd "$OUT/project"
    ./gradlew --no-daemon -q "$BUILD" -PappId="$ID" -PappName="$NAME"
)
apk=$(find "$OUT/project/app/build/outputs/apk" -name '*.apk' | head -n 1)
[ -n "$apk" ] || {
    echo "build-android-app: Gradle produced no APK" >&2
    exit 1
}
cp "$apk" "$OUT/app.apk"
echo "apk: $OUT/app.apk"
