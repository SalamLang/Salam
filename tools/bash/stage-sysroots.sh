#!/bin/sh
# Copies this host's musl and mingw-w64 CRT/import libraries into a staging
# directory laid out by target triple, ready for tools/bash/build-embed.sh to
# tar into a self-contained salam.
#
# This is c/Makefile's `stage-sysroots` target. Same file lists, same search
# order, same "missing is fine, incomplete is an error" rule: a toolchain that
# is not installed is skipped with a note, but one that is installed and half
# there fails, because that produces a sysroot that links most programs and
# then breaks on the one that needs the missing piece.
#
# Usage:
#   tools/bash/stage-sysroots.sh [--out DIR] [--musl-arch ARCH]
#
# Produces, under DIR:
#   <arch>-linux-musl/     crt1.o crti.o crtn.o libc.a (+ compiler-rt builtins)
#   x86_64-w64-mingw32/lib CRT, gcc runtime, and the import libraries
#   i686-w64-mingw32/lib   the 32-bit counterpart

set -eu

OUT=$(pwd)/sysroots
MUSL_ARCH=x86_64

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
    --musl-arch)
        MUSL_ARCH=$2
        shift 2
        ;;
    --musl-arch=*)
        MUSL_ARCH=${1#*=}
        shift
        ;;
    -h | --help)
        sed -n '2,20p' "$0"
        exit 0
        ;;
    *)
        echo "unknown argument: $1" >&2
        exit 2
        ;;
    esac
done

echo "Staging sysroots into $OUT ..."

# ---- musl ------------------------------------------------------------------
MUSL_SR="$OUT/$MUSL_ARCH-linux-musl"
if [ -f "/usr/lib/$MUSL_ARCH-linux-musl/crt1.o" ]; then
    mkdir -p "$MUSL_SR"
    for f in crt1.o crti.o crtn.o libc.a; do
        cp "/usr/lib/$MUSL_ARCH-linux-musl/$f" "$MUSL_SR/"
    done
    # compiler-rt's builtins cover the helpers libgcc would otherwise supply.
    # Optional: a musl sysroot without them still links most programs.
    rt=$(find /usr/lib -path "*/lib/clang/*/lib/linux/libclang_rt.builtins-$MUSL_ARCH.a" \
        2>/dev/null | sort | head -1)
    if [ -n "$rt" ]; then
        cp "$rt" "$MUSL_SR/"
    else
        echo "  (compiler-rt builtins not bundled; optional)"
    fi
    echo "  musl  -> $MUSL_SR"
else
    echo "  (musl not found - skipped; 'apt install musl-dev' to cross-compile Linux)"
fi

# ---- mingw -----------------------------------------------------------------
# Where these live differs by host: a Linux cross-build keeps them under
# /usr/<triple>, an MSYS2 host directly in /mingw64/lib. Both are searched so
# this works natively on Windows too.
MINGW_CRT="crt2.o dllcrt2.o libmingw32.a libmingwex.a libmoldname.a libmsvcrt.a
           libadvapi32.a libshell32.a libuser32.a libkernel32.a"
# Beyond the bare CRT: what `link dynamic "ws2_32"` and friends resolve against.
MINGW_EXTRA="libws2_32.a libgdi32.a libole32.a liboleaut32.a libuuid.a
             libcomdlg32.a libwinspool.a libpsapi.a libiphlpapi.a
             libcrypt32.a libbcrypt.a libsecur32.a libwinmm.a
             libversion.a libnetapi32.a libuserenv.a libdbghelp.a"

stage_mingw() {
    arch=$1
    shift
    sr="$OUT/$arch-w64-mingw32"
    src=
    for d in "$@"; do
        if [ -f "$d/crt2.o" ]; then
            src=$d
            break
        fi
    done
    if [ -z "$src" ]; then
        echo "  ($arch mingw not found - skipped; 'apt install mingw-w64', or MSYS2's mingw-w64 packages)"
        return 0
    fi
    mkdir -p "$sr/lib"
    for x in $MINGW_CRT; do
        cp "$src/$x" "$sr/lib/" || {
            echo "  ! incomplete mingw sysroot in $src ($x)" >&2
            exit 1
        }
    done
    for x in $MINGW_EXTRA; do
        cp "$src/$x" "$sr/lib/" 2>/dev/null || true
    done
    g=
    for cand in /usr/lib/gcc/"$arch"-w64-mingw32/*win32 /usr/lib/gcc/"$arch"-w64-mingw32/* \
        /mingw64/lib/gcc/"$arch"-w64-mingw32/* /mingw32/lib/gcc/"$arch"-w64-mingw32/* \
        /c/msys64/mingw64/lib/gcc/"$arch"-w64-mingw32/* /c/msys64/mingw32/lib/gcc/"$arch"-w64-mingw32/*; do
        if [ -f "$cand/crtbegin.o" ]; then
            g=$cand
            break
        fi
    done
    [ -n "$g" ] || {
        echo "  ! gcc-mingw runtime (crtbegin.o/libgcc.a) not found for $arch" >&2
        exit 1
    }
    cp "$g/crtbegin.o" "$g/crtend.o" "$g/libgcc.a" "$g/libgcc_eh.a" "$sr/lib/"
    echo "  mingw -> $sr (from $src)"
}

stage_mingw x86_64 /usr/x86_64-w64-mingw32/lib /mingw64/lib /clang64/lib /ucrt64/lib /c/msys64/mingw64/lib
stage_mingw i686 /usr/i686-w64-mingw32/lib /mingw32/lib /c/msys64/mingw32/lib
