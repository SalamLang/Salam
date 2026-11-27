#!/bin/sh
set -eu

LLVM_TAG=llvmorg-22.1.8
LLVM_TARGETS="X86;AArch64;ARM;RISCV;WebAssembly"

[ $# -eq 3 ] || {
    echo "usage: $0 <i686|armhf> <musl toolchain dir> <install prefix>" >&2
    exit 2
}
ARCH=$1
TC=$2
PREFIX=$3

case $ARCH in
i686)
    TRIPLE=i686-unknown-linux-musl
    DEFAULT_TRIPLE=i686-pc-linux-gnu
    PROCESSOR=i686
    ARCH_FLAGS=
    ;;
armhf)
    TRIPLE=arm-unknown-linux-musleabihf
    DEFAULT_TRIPLE=arm-unknown-linux-gnueabihf
    PROCESSOR=arm
    ARCH_FLAGS="-march=armv6 -mfpu=vfp -mfloat-abi=hard -marm"
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
"$GCC" -v 2>&1 | tail -3

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT INT TERM

git clone --depth 1 --branch "$LLVM_TAG" --filter=blob:none --sparse \
    https://github.com/llvm/llvm-project.git "$WORK/src"
git -C "$WORK/src" sparse-checkout set llvm lld cmake third-party libunwind

cmake -G Ninja -S "$WORK/src/llvm" -B "$WORK/native" \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_TARGETS_TO_BUILD=X86 \
    -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_INCLUDE_BENCHMARKS=OFF
ninja -C "$WORK/native" llvm-tblgen

cmake -G Ninja -S "$WORK/src/llvm" -B "$WORK/cross" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR="$PROCESSOR" \
    -DCMAKE_C_COMPILER="${CROSS}gcc" \
    -DCMAKE_CXX_COMPILER="${CROSS}g++" \
    -DCMAKE_ASM_COMPILER="${CROSS}gcc" \
    -DCMAKE_AR="${CROSS}ar" \
    -DCMAKE_RANLIB="${CROSS}ranlib" \
    -DCMAKE_C_FLAGS="$ARCH_FLAGS" \
    -DCMAKE_CXX_FLAGS="$ARCH_FLAGS" \
    -DCMAKE_ASM_FLAGS="$ARCH_FLAGS" \
    -DCMAKE_EXE_LINKER_FLAGS=-static \
    -DLLVM_TABLEGEN="$WORK/native/bin/llvm-tblgen" \
    -DLLVM_HOST_TRIPLE="$DEFAULT_TRIPLE" \
    -DLLVM_DEFAULT_TARGET_TRIPLE="$DEFAULT_TRIPLE" \
    -DLLVM_ENABLE_PROJECTS=lld \
    -DLLVM_TARGETS_TO_BUILD="$LLVM_TARGETS" \
    -DLLVM_BUILD_TOOLS=OFF -DLLVM_BUILD_UTILS=OFF -DLLD_BUILD_TOOLS=OFF \
    -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_INCLUDE_BENCHMARKS=OFF \
    -DLLVM_ENABLE_ZLIB=OFF -DLLVM_ENABLE_ZSTD=OFF -DLLVM_ENABLE_LIBXML2=OFF \
    -DLLVM_ENABLE_TERMINFO=OFF -DLLVM_ENABLE_LIBEDIT=OFF -DLLVM_ENABLE_LIBPFM=OFF \
    -DLLVM_ENABLE_PIC=OFF -DBUILD_SHARED_LIBS=OFF -DLLVM_ENABLE_ASSERTIONS=OFF
ninja -C "$WORK/cross" install

for f in lib/liblldCommon.a lib/liblldELF.a lib/libLLVMCore.a include/lld/Common/Driver.h \
    include/llvm/Config/llvm-config.h; do
    [ -f "$PREFIX/$f" ] || {
        echo "error: the LLVM install has no $f" >&2
        exit 1
    }
done
du -sh "$PREFIX/lib" "$PREFIX/include"
