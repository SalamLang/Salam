#!/bin/sh
# Packages a compiler that the native backend built, laid out like the
# release archives: salam (salam.exe on Windows) with std/ next to it, plus
# README.md, LICENSE and a BUILD.txt naming the commit and target. These
# packages are for internal testing of the native backend, not releases.
#
# Windows packages also carry the mingw-w64 runtime the built-in linker needs
# (CRT objects, the compiler runtime library and the import libraries std
# links) under sysroots/<arch>-w64-mingw32/lib, where the compiler looks next
# to itself, so they build programs with nothing else installed. For x86_64
# it comes from the mingw-w64 packages (/usr/x86_64-w64-mingw32 and
# /usr/lib/gcc/x86_64-w64-mingw32), for aarch64 from the llvm-mingw tree in
# SALAM_LLVM_MINGW.
#
# The result is <out dir>/salam-native-<platform>/ and the same folder as
# <out dir>/salam-native-<platform>.tar.gz, which keeps the executable bit.
#
# Usage: tools/bash/package-native.sh <compiler> <triple> <platform> <out dir>
#   e.g. tools/bash/package-native.sh out/salam.exe x86_64-w64-windows-gnu windows-x64 dist

set -eu
BIN=$1
TRIPLE=$2
PLATFORM=$3
OUT=$4
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
NAME=salam-native-$PLATFORM
EXE=salam
case "$TRIPLE" in *windows* | *mingw*) EXE=salam.exe ;; esac
if [ ! -f "$BIN" ]; then
    echo "package-native: no compiler at $BIN" >&2
    exit 1
fi
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)
PKG=$OUT/$NAME
rm -rf "$PKG" "$PKG.tar.gz"
mkdir -p "$PKG"
cp "$BIN" "$PKG/$EXE"
chmod +x "$PKG/$EXE"
cp -R "$ROOT/std" "$PKG/std"
cp "$ROOT/README.md" "$ROOT/LICENSE" "$PKG/" 2>/dev/null || true

WINLIBS="crt2.o crtbegin.o crtend.o libmingw32.a libmingwex.a libmsvcrt.a libucrt.a libmoldname.a
libkernel32.a libadvapi32.a libshell32.a libuser32.a libws2_32.a libole32.a libgdi32.a libcomdlg32.a
libsecur32.a libcrypt32.a libbcrypt.a libiphlpapi.a libwinhttp.a libpsapi.a libunwind.a libuuid.a
libntdll.a libversion.a libdbghelp.a libwinmm.a libuserenv.a libshlwapi.a libcomctl32.a
libsetupapi.a libwininet.a libncrypt.a libgcc.a libgcc_eh.a"
bundle() {
    lib=$PKG/sysroots/$1-w64-mingw32/lib
    shift
    mkdir -p "$lib"
    for f in $WINLIBS; do
        for d in "$@"; do
            if [ -f "$d/$f" ]; then
                cp "$d/$f" "$lib/"
                break
            fi
        done
    done
    for need in crt2.o crtbegin.o crtend.o libmingw32.a libmingwex.a libmsvcrt.a libkernel32.a; do
        if [ ! -f "$lib/$need" ]; then
            echo "package-native: no $need in $*" >&2
            exit 1
        fi
    done
}
case "$TRIPLE" in
x86_64*windows* | x86_64*mingw*)
    gcc=$(find /usr/lib/gcc/x86_64-w64-mingw32 -name crtbegin.o 2>/dev/null | sort | tail -1)
    if [ -z "$gcc" ]; then
        echo "package-native: install mingw-w64 for the x86_64 Windows runtime" >&2
        exit 1
    fi
    bundle x86_64 "$(dirname "$gcc")" /usr/x86_64-w64-mingw32/lib
    ;;
aarch64*windows* | aarch64*mingw*)
    LM=${SALAM_LLVM_MINGW:?set SALAM_LLVM_MINGW to an llvm-mingw tree}
    rt=$(find "$LM/lib/clang" -name libclang_rt.builtins-aarch64.a -path "*windows*" | sort | tail -1)
    if [ -z "$rt" ]; then
        echo "package-native: no libclang_rt.builtins-aarch64.a under $LM/lib/clang" >&2
        exit 1
    fi
    bundle aarch64 "$LM/aarch64-w64-mingw32/lib"
    cp "$rt" "$PKG/sysroots/aarch64-w64-mingw32/lib/"
    ;;
esac

{
    echo "Salam compiler built by Salam's native backend (--backend=native, built-in linker)."
    echo "For internal testing of the native backend; this is not a release."
    echo ""
    echo "target:  $TRIPLE"
    echo "commit:  $(git -C "$ROOT" rev-parse HEAD 2>/dev/null || echo unknown)"
    echo "version: $(cat "$ROOT/VERSION" 2>/dev/null || echo unknown)"
    echo "built:   $(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo ""
    echo "Unpack it anywhere and run the compiler from the folder; std/ next to it is found"
    echo "on its own:"
    echo ""
    echo "  ./$EXE version"
    echo "  ./$EXE build app.salam --backend=native -o app"
} >"$PKG/BUILD.txt"
tar -czf "$PKG.tar.gz" -C "$OUT" "$NAME"
echo "packaged $NAME in $PKG and $PKG.tar.gz ($(du -sh "$PKG.tar.gz" | cut -f1))"
