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

cd "$WORK"
fetch "musl-cross.tgz" \
    "https://musl.cc/${MUSL_TRIPLE}-cross.tgz" \
    "https://more.musl.cc/11/x86_64-linux-musl/${MUSL_TRIPLE}-cross.tgz"
tar xzf musl-cross.tgz
MUSL_TOOLCHAIN="$WORK/${MUSL_TRIPLE}-cross"
MUSL_SYSROOT="$MUSL_TOOLCHAIN/$MUSL_TRIPLE"
export PATH="$MUSL_TOOLCHAIN/bin:$PATH"
"${MUSL_TRIPLE}-gcc" --version | head -1

git clone --filter=blob:none https://github.com/TinyCC/tinycc.git tinycc
cd tinycc
git checkout "$TINYCC_REF"

mkdir build
gcc -DC2STR conftest.c -o build/c2str.exe
cd build

# shellcheck disable=SC2086
../configure --cross-prefix="${MUSL_TRIPLE}-" --cpu="$TCC_CPU" \
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
find "$MUSL_TOOLCHAIN/lib/gcc/$MUSL_TRIPLE" -name libgcc.a -exec cp {} "$DIST/musl/lib/" \;
cp -r "$MUSL_SYSROOT/include/"* "$DIST/musl/include/"

echo "bundle contents:"
find "$DIST" -maxdepth 2
