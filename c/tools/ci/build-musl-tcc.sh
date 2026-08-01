#!/bin/sh
# Cross-builds a statically musl-linked tcc for one architecture and stages
# it as a self-contained bundle at /tmp/tcc-bundle/tcc, ready to sit next to
# a salam binary (salam_find_bundled_tool() looks for exedir/tcc/tcc).
#
# Params come in via env: MUSL_TRIPLE, TCC_CPU, TCC_CONFIGURE_EXTRA, TINYCC_REF

set -eu

: "${MUSL_TRIPLE:?}"
: "${TCC_CPU:?}"
: "${TCC_CONFIGURE_EXTRA:=}"
: "${TINYCC_REF:?}"

WORK=/tmp/tcc-build-$MUSL_TRIPLE
BUNDLE=/tmp/tcc-bundle
rm -rf "$WORK" "$BUNDLE"
mkdir -p "$WORK" "$BUNDLE/tcc/musl/lib" "$BUNDLE/tcc/musl/include" "$BUNDLE/tcc/include"

fetch() {
    out="$1"
    shift
    for url in "$@"; do
        echo "fetching $url"
        if curl -fL --connect-timeout 10 --max-time 900 --retry 3 --retry-delay 5 \
            -o "$out" "$url"; then
            return 0
        fi
    done
    echo "ERROR: could not fetch $out from any mirror" >&2
    return 1
}

# cross-tools/musl-cross publishes the same prebuilt toolchains as GitHub
# Releases, which resolve on the same network GitHub-hosted runners already
# use; musl.cc has been observed unreachable (connection timeout) from
# GitHub-hosted runners, so it is kept only as a fallback mirror.
# cross-tools/musl-cross names its targets with an "unknown" vendor
# component (e.g. i686-unknown-linux-musl) where musl.cc/MUSL_TRIPLE omits
# it (i686-linux-musl); derive the former from the latter unless overridden.
: "${MUSL_CTRIPLE:=$(printf '%s' "$MUSL_TRIPLE" | sed 's/-linux-/-unknown-linux-/')}"

cd "$WORK"
fetch "musl-cross.tar" \
    "https://github.com/cross-tools/musl-cross/releases/latest/download/${MUSL_CTRIPLE}.tar.xz" \
    "https://musl.cc/${MUSL_TRIPLE}-cross.tgz" \
    "https://more.musl.cc/11/x86_64-linux-musl/${MUSL_TRIPLE}-cross.tgz"
mkdir -p muslcross-x
tar xf musl-cross.tar -C muslcross-x
# musl.cc puts the cross gcc directly at <toolchain>/bin/<triple>-gcc;
# cross-tools/musl-cross nests an extra <triple>/ level and also has an
# inner bin/ with unprefixed-vendor binutils only (no gcc) - so locate the
# toolchain root via the actual *-gcc binary rather than any "bin" dir.
GCC_BIN="$(find "$WORK/muslcross-x" -maxdepth 4 -type f -name '*-gcc' | head -1)"
LIBC_A="$(find "$WORK/muslcross-x" -type f -name libc.a | head -1)"
: "${GCC_BIN:?could not locate cross gcc binary in extracted musl toolchain}"
: "${LIBC_A:?could not locate libc.a in extracted musl toolchain}"
MUSL_TOOLCHAIN="$(dirname "$(dirname "$GCC_BIN")")"
CROSS_PREFIX="$(basename "$GCC_BIN" gcc)"
# libc.a always sits at <X>/lib/libc.a, where <X> also has an include/
# sibling (musl.cc: <X> is the triple dir; cross-tools/musl-cross: <X> is
# the sysroot's usr/ dir). Two dirname hops from libc.a lands on <X>,
# which is what the rest of this script expects MUSL_SYSROOT to be.
MUSL_SYSROOT="$(dirname "$(dirname "$LIBC_A")")"
export PATH="$MUSL_TOOLCHAIN/bin:$PATH"
"${CROSS_PREFIX}gcc" --version | head -1

git clone --filter=blob:none https://github.com/TinyCC/tinycc.git tinycc
cd tinycc
git checkout "$TINYCC_REF"

mkdir build
gcc -DC2STR conftest.c -o build/c2str.exe
cd build

# shellcheck disable=SC2086
../configure --cross-prefix="$CROSS_PREFIX" --cpu="$TCC_CPU" \
    --extra-ldflags=-static --config-musl $TCC_CONFIGURE_EXTRA

make BCHECK_O= "XFLAGS-unx=-B../.. -I../.. -I$MUSL_SYSROOT/include" -j"$(nproc)"

file tcc
./tcc -v

DIST="$BUNDLE/tcc"
cp tcc "$DIST/tcc"
cp libtcc1.a "$DIST/libtcc1.a"
cp ../include/*.h ../tcclib.h "$DIST/include/"
cp "$MUSL_SYSROOT/lib/crt1.o" "$MUSL_SYSROOT/lib/crti.o" "$MUSL_SYSROOT/lib/crtn.o" \
    "$MUSL_SYSROOT/lib/libc.a" "$MUSL_SYSROOT/lib/libm.a" "$DIST/musl/lib/"
find "$MUSL_TOOLCHAIN" -name libgcc.a -exec cp {} "$DIST/musl/lib/" \;
cp -r "$MUSL_SYSROOT/include/"* "$DIST/musl/include/"

echo "bundle contents:"
find "$DIST" -maxdepth 2
