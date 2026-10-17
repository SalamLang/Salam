#!/bin/sh
# Generates the C sources of the mobile engine (compiler/mobile_main.salam)
# and, with --std, a minified copy of std/ for the apps to ship.
#
# Usage:
#   tools/bash/build-mobile-engine.sh --out DIR [--target TRIPLE] [--std DIR]
#
# The C lands in DIR/gen. Files whose content did not change keep their
# timestamp, so CMake and Xcode only recompile what really changed. The
# compiler runs in a scratch directory because a build from the repository
# root wipes .salam-build.

set -eu
. "$(dirname "$0")/lib.sh"
salam_ensure_compiler

ROOT=$(pwd)
case $SALAM in
/*) ;;
*) SALAM="$ROOT/$SALAM" ;;
esac

OUT=
TARGET=
STD_OUT=
while [ $# -gt 0 ]; do
    case $1 in
    --out)
        OUT=$2
        shift 2
        ;;
    --out=*)
        OUT=${1#*=}
        shift
        ;;
    --target)
        TARGET=$2
        shift 2
        ;;
    --target=*)
        TARGET=${1#*=}
        shift
        ;;
    --std)
        STD_OUT=$2
        shift 2
        ;;
    --std=*)
        STD_OUT=${1#*=}
        shift
        ;;
    *)
        echo "build-mobile-engine: unknown argument '$1'" >&2
        exit 2
        ;;
    esac
done
[ -n "$OUT" ] || {
    echo "build-mobile-engine: --out DIR is required" >&2
    exit 2
}

WORK=$(mktemp -d "${TMPDIR:-/tmp}/salam-mobile.XXXXXX")
trap 'rm -rf "$WORK"' EXIT INT TERM

(
    cd "$WORK"
    if [ -n "$TARGET" ]; then
        "$SALAM" build --backend=c --emit-c --target="$TARGET" --stdlib-path="$ROOT/std" \
            "$ROOT/compiler/mobile_main.salam" --output="$WORK/engine" --log-level=warn
    else
        "$SALAM" build --backend=c --emit-c --stdlib-path="$ROOT/std" \
            "$ROOT/compiler/mobile_main.salam" --output="$WORK/engine" --log-level=warn
    fi
)

GEN="$WORK/.salam-build"
[ -n "$(find "$GEN" -name '*.c' 2>/dev/null | head -n 1)" ] || {
    echo "build-mobile-engine: the compiler produced no C" >&2
    exit 1
}

mkdir -p "$OUT/gen"
for f in "$GEN"/*.c "$GEN"/*.h; do
    [ -f "$f" ] || continue
    dst="$OUT/gen/$(basename "$f")"
    if ! cmp -s "$f" "$dst" 2>/dev/null; then
        cp "$f" "$dst"
    fi
done
for f in "$OUT"/gen/*; do
    [ -f "$f" ] || continue
    [ -f "$GEN/$(basename "$f")" ] || rm -f "$f"
done
echo "engine C for ${TARGET:-host} in $OUT/gen"

if [ -n "$STD_OUT" ]; then
    rm -rf "$STD_OUT"
    mkdir -p "$STD_OUT"
    (
        cd "$ROOT/std"
        find . -name '*.salam' | while IFS= read -r f; do
            mkdir -p "$STD_OUT/$(dirname "$f")"
            cp "$f" "$STD_OUT/$f"
        done
    )
    "$SALAM" format --minify -r "$STD_OUT" >/dev/null
    echo "minified std in $STD_OUT"
fi
