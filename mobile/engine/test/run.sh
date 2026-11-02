#!/bin/sh
# Builds the mobile engine for this machine and runs its C API test.
#
# Usage: mobile/engine/test/run.sh

set -eu
HERE=$(cd "$(dirname "$0")/.." && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
BUILD="$HERE/build"

sh "$ROOT/tools/bash/build-mobile-engine.sh" --out "$BUILD"
cmake -S "$HERE" -B "$BUILD/host" -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo >/dev/null
cmake --build "$BUILD/host"
WORK=$(mktemp -d "${TMPDIR:-/tmp}/salam-engine-test.XXXXXX")
trap 'rm -rf "$WORK"' EXIT INT TERM
echo Ali | "$BUILD/host/salam_engine_test" "$ROOT/std" "$WORK"
