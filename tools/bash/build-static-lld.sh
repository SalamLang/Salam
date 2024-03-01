#!/bin/sh

set -eu

PREFIX=
LLVM_CONFIG=${LLVM_CONFIG:-llvm-config}
JOBS=

while [ $# -gt 0 ]; do
    case "$1" in
    --prefix)
        PREFIX=$2
        shift 2
        ;;
    --prefix=*)
        PREFIX=${1#--prefix=}
        shift
        ;;
    --llvm-config)
        LLVM_CONFIG=$2
        shift 2
        ;;
    --llvm-config=*)
        LLVM_CONFIG=${1#--llvm-config=}
        shift
        ;;
    --jobs)
        JOBS=$2
        shift 2
        ;;
    --jobs=*)
        JOBS=${1#--jobs=}
        shift
        ;;
    *)
        echo "unknown argument: $1" >&2
        exit 2
        ;;
    esac
done

[ -n "$PREFIX" ] || {
    echo "usage: $0 --prefix DIR [--llvm-config PATH] [--jobs N]" >&2
    exit 2
}

for t in cmake curl tar; do
    command -v "$t" >/dev/null 2>&1 || {
        echo "error: $t is required" >&2
        exit 2
    }
done
command -v "$LLVM_CONFIG" >/dev/null 2>&1 || {
    echo "error: no llvm-config at '$LLVM_CONFIG'" >&2
    exit 2
}

VER=$("$LLVM_CONFIG" --version | tr -d '[:space:]')
VER=$(printf '%s' "$VER" | sed 's/[^0-9.].*$//')
case "$VER" in
[0-9]*.[0-9]*.[0-9]*) ;;
*)
    echo "error: cannot parse an LLVM release version out of '$VER'" >&2
    exit 2
    ;;
esac

if [ -z "$JOBS" ]; then
    JOBS=$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 2)
fi

if [ -f "$PREFIX/lib/liblldCommon.a" ]; then
    echo "static lld $VER already at $PREFIX"
    exit 0
fi

WORK=$(mktemp -d 2>/dev/null || mktemp -d -t salam-lld)
trap 'rm -rf "$WORK"' EXIT INT TERM

BASE="https://github.com/llvm/llvm-project/releases/download/llvmorg-$VER"
echo "fetching lld $VER sources"
for p in lld cmake; do
    curl -fsSL "$BASE/$p-$VER.src.tar.xz" -o "$WORK/$p.tar.xz" || {
        echo "error: cannot download $p-$VER.src.tar.xz from $BASE" >&2
        exit 1
    }
    tar -xJf "$WORK/$p.tar.xz" -C "$WORK"
    mv "$WORK/$p-$VER.src" "$WORK/$p"
done

echo "configuring lld $VER (static, standalone) -> $PREFIX"
cmake -S "$WORK/lld" -B "$WORK/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DBUILD_SHARED_LIBS=OFF \
    -DLLD_BUILT_STANDALONE=ON \
    -DLLVM_INCLUDE_TESTS=OFF \
    -DLLD_INCLUDE_TESTS=OFF \
    -DLLVM_DIR="$("$LLVM_CONFIG" --cmakedir)" \
    -DLLVM_CONFIG_PATH="$(command -v "$LLVM_CONFIG")"

cmake --build "$WORK/build" -j "$JOBS"
cmake --install "$WORK/build"

[ -f "$PREFIX/lib/liblldCommon.a" ] || {
    echo "error: the build produced no $PREFIX/lib/liblldCommon.a" >&2
    exit 1
}
echo "static lld $VER installed:"
ls "$PREFIX"/lib/liblld*.a
