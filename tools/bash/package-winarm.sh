#!/bin/sh
# Packages Salam for Windows on Arm: a prebuilt aarch64 salam.exe,
# the standard library, and the part of llvm-mingw's aarch64 runtime the
# built-in linker needs (CRT objects, the mingw and UCRT import libraries and
# compiler-rt's builtins), under sysroots/aarch64-w64-mingw32/lib where the
# compiler looks next to itself. The result needs nothing else installed.
#
# Usage: tools/bash/package-winarm.sh <salam.exe> <llvm-mingw root> <out dir>

set -eu
EXE=$1
LM=$2
OUT=$3
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
SRC="$LM/aarch64-w64-mingw32/lib"
RT=$(find "$LM/lib/clang" -name libclang_rt.builtins-aarch64.a -path "*windows*" | sort | tail -1)
for f in crt2.o crtbegin.o crtend.o libmingw32.a libmingwex.a libmsvcrt.a libkernel32.a; do
    if [ ! -f "$SRC/$f" ]; then
        echo "package-winarm: $SRC has no $f; is $LM an llvm-mingw tree?" >&2
        exit 1
    fi
done
if [ ! -f "$EXE" ]; then
    echo "package-winarm: no salam.exe at $EXE" >&2
    exit 1
fi
if [ -z "$RT" ]; then
    echo "package-winarm: no libclang_rt.builtins-aarch64.a under $LM/lib/clang" >&2
    exit 1
fi
mkdir -p "$OUT/sysroots/aarch64-w64-mingw32/lib"
cp "$EXE" "$OUT/salam.exe"
rm -rf "$OUT/std"
cp -R "$ROOT/std" "$OUT/std"
for f in crt2.o crtbegin.o crtend.o libmingw32.a libmingwex.a libmsvcrt.a libucrt.a \
    libmoldname.a libkernel32.a libadvapi32.a libshell32.a libuser32.a libws2_32.a \
    libole32.a libgdi32.a libcomdlg32.a libsecur32.a libcrypt32.a libbcrypt.a \
    libiphlpapi.a libwinhttp.a libpsapi.a libunwind.a libuuid.a libntdll.a libversion.a \
    libdbghelp.a libwinmm.a libuserenv.a libshlwapi.a libcomctl32.a libsetupapi.a \
    libwininet.a libncrypt.a libimm32.a libpowrprof.a libm.a libpthread.a; do
    if [ -f "$SRC/$f" ]; then
        cp "$SRC/$f" "$OUT/sysroots/aarch64-w64-mingw32/lib/"
    fi
done
cp "$RT" "$OUT/sysroots/aarch64-w64-mingw32/lib/"
cp "$RT" "$OUT/sysroots/aarch64-w64-mingw32/lib/libgcc.a"
"$LM/bin/llvm-ar" rcs "$OUT/sysroots/aarch64-w64-mingw32/lib/libgcc_eh.a"
cp "$ROOT/LICENSE" "$OUT/" 2>/dev/null || true
echo "packaged Salam for Windows on Arm in $OUT"
