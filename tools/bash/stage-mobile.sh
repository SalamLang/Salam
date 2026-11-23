#!/bin/sh
# Copies the mobile build files into a release folder, laid out as in the
# source tree so tools/bash/build-android-app.sh finds them from the folder
# that holds std/.
#
# Usage: tools/bash/stage-mobile.sh DIST

set -eu
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
DIST=$1
mkdir -p "$DIST/tools/bash" "$DIST/mobile/host" "$DIST/android"
cp "$ROOT/tools/bash/build-android-app.sh" "$ROOT/tools/bash/build-ios-app.sh" "$DIST/tools/bash/"
cp -R "$ROOT/mobile/host/android" "$ROOT/mobile/host/common" "$ROOT/mobile/host/ios" "$DIST/mobile/host/"
cp -R "$ROOT/android/gradle" "$ROOT/android/gradlew" "$DIST/android/"
