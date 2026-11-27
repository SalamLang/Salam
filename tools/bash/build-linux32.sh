#!/bin/sh
set -eu

[ $# -eq 5 ] || {
    echo "usage: $0 <i686|armhf> <seed salam> <musl toolchain dir> <llvm prefix> <work dir>" >&2
    exit 2
}
ARCH=$1
SEED=$2
TC=$3
LLVM_PREFIX=$4
WORK=$5
ROOT=$(cd "$(dirname "$0")/../.." && pwd)

case $ARCH in
i686)
    TRIPLE=i686-unknown-linux-musl
    TARGET=i686-linux-musl
    EMBED_OPT=--musl-i686
    ;;
armhf)
    TRIPLE=arm-unknown-linux-musleabihf
    TARGET=arm-linux-musleabihf
    EMBED_OPT=--musl-arm
    ;;
*)
    echo "error: unknown architecture '$ARCH'" >&2
    exit 2
    ;;
esac

GCC=$(find "$TC" -name "$TRIPLE-gcc" \( -type f -o -type l \) | head -1)
[ -n "$GCC" ] || {
    echo "error: no $TRIPLE-gcc under $TC" >&2
    exit 1
}
CROSS=${GCC%gcc}

need() {
    found=$("$GCC" -print-file-name="$1")
    [ -f "$found" ] || {
        echo "error: the $TRIPLE toolchain has no $1" >&2
        exit 1
    }
    printf '%s\n' "$found"
}

mkdir -p "$WORK"
WORK=$(cd "$WORK" && pwd)
LIBS=$WORK/libs
OWN_SYSROOT=$WORK/sysroots/$TARGET
rm -rf "$LIBS" "$OWN_SYSROOT"
mkdir -p "$LIBS" "$OWN_SYSROOT" "$WORK/llvm" "$WORK/embed"

for f in crt1.o crti.o crtn.o libc.a libgcc.a libgcc_eh.a; do
    path=$(need "$f")
    cp "$path" "$OWN_SYSROOT/"
done

cxx=$(need libstdc++.a)
cp "$cxx" "$WORK/cxx.a"
printf 'void *__dso_handle __attribute__((visibility("hidden"))) = 0;\n' >"$WORK/dso_handle.c"
"$GCC" -O2 -c "$WORK/dso_handle.c" -o "$WORK/dso_handle.o"
{
    echo "create $LIBS/merged.a"
    echo "addlib $WORK/cxx.a"
    echo "addlib $OWN_SYSROOT/libgcc_eh.a"
    echo "addlib $OWN_SYSROOT/libgcc.a"
    atomic=$("$GCC" -print-file-name=libatomic.a)
    if [ -f "$atomic" ]; then
        echo "addlib $atomic"
    fi
    echo "addmod $WORK/dso_handle.o"
    echo "save"
    echo "end"
} >"$WORK/merge.mri"
"${CROSS}ar" -M <"$WORK/merge.mri"
mv "$LIBS/merged.a" "$LIBS/libstdc++.a"
"${CROSS}ar" rcs "$LIBS/libz.a"
"${CROSS}ar" rcs "$LIBS/libzstd.a"

cat >"$WORK/llvm-config" <<'LLVM_CONFIG'
#!/bin/sh
set -eu
for arg in "$@"; do
    case $arg in
    --includedir) echo "$SALAM_LLVM_PREFIX/include" ;;
    --libdir) echo "$SALAM_LLVM_PREFIX/lib" ;;
    --bindir) echo "$SALAM_LLVM_PREFIX/bin" ;;
    --version)
        sed -n 's/^#define LLVM_VERSION_STRING "\(.*\)"/\1/p' \
            "$SALAM_LLVM_PREFIX/include/llvm/Config/llvm-config.h"
        ;;
    --cxxflags)
        echo "-I$SALAM_LLVM_PREFIX/include -std=c++17 -fno-exceptions -fno-rtti -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS"
        ;;
    --system-libs) echo "-lpthread -ldl -lm" ;;
    --libs)
        for f in "$SALAM_LLVM_PREFIX"/lib/lib*.a; do
            b=${f##*/}
            b=${b%.a}
            printf -- '-l%s ' "${b#lib}"
        done
        echo
        ;;
    esac
done
LLVM_CONFIG
chmod +x "$WORK/llvm-config"

SALAM_LLVM_PREFIX=$LLVM_PREFIX WITH_LLD=1 CC=$GCC CXX=${CROSS}g++ AR=${CROSS}ar \
    sh "$ROOT/std/llvm/native/build.sh" --llvm-config "$WORK/llvm-config" --out "$WORK/llvm"

sh "$ROOT/tools/bash/fetch-musl-sysroots.sh" --out "$WORK/muslcc" --arches x86_64
sh "$ROOT/tools/bash/stage-sysroots.sh" --out="$WORK/sysroots"
sh "$ROOT/tools/bash/stage-wasi-sysroot.sh" --out="$WORK/sysroots/wasm32-wasi"
for f in "$WORK/muslcc/sr-x86_64/crt1.o" "$WORK/sysroots/x86_64-w64-mingw32/lib/crt2.o" \
    "$WORK/sysroots/wasm32-wasi/lib/libc.a"; do
    [ -f "$f" ] || {
        echo "error: a sysroot to embed is missing: $f" >&2
        exit 1
    }
done

CC=$GCC AR=${CROSS}ar sh "$ROOT/tools/bash/build-embed.sh" --out "$WORK/embed" \
    "$EMBED_OPT" "$OWN_SYSROOT" \
    --musl-x86_64 "$WORK/muslcc/sr-x86_64" \
    --mingw-x86_64 "$WORK/sysroots/x86_64-w64-mingw32" \
    --wasi "$WORK/sysroots/wasm32-wasi"

rm -f "$ROOT/salam"
sh "$ROOT/tools/bash/build-selfhost.sh" --seed="$SEED" --seed-is-builder \
    --llvm="$WORK/llvm" --embed="$WORK/embed" --output="$ROOT/salam" \
    -- --target="$TARGET" --linker=system --libpath="$LIBS"

"${CROSS}readelf" -h "$ROOT/salam"
if "${CROSS}readelf" -l "$ROOT/salam" | grep -q INTERP; then
    echo "error: $ROOT/salam is not statically linked" >&2
    exit 1
fi
ls -l "$ROOT/salam"
