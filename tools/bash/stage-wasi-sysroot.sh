#!/bin/sh
# Stages the minimal wasm32-wasi sysroot salam needs to link WASI programs
# (Cloudflare Workers included) with its in-process LLD, so no wasi-sdk install
# is required. The files come from a local wasi-sdk ($WASI_SDK_PATH or --sdk)
# when one is present, otherwise from the wasi-sdk GitHub release.
#
# Usage:
#   tools/bash/stage-wasi-sysroot.sh --out DIR [--sdk DIR] [--version 34]
#
# Produces DIR/lib/{crt1-command.o,libc.a,libwasi-emulated-signal.a,
# libclang_rt.builtins.a} and DIR/NOTICE.

set -eu

OUT=
SDK=${WASI_SDK_PATH:-}
VER=34

while [ $# -gt 0 ]; do
    case $1 in
    --out)
        OUT=$2
        shift 2
        ;;
    --out=*)
        OUT=${1#--out=}
        shift
        ;;
    --sdk)
        SDK=$2
        shift 2
        ;;
    --sdk=*)
        SDK=${1#--sdk=}
        shift
        ;;
    --version)
        VER=$2
        shift 2
        ;;
    --version=*)
        VER=${1#--version=}
        shift
        ;;
    *)
        echo "unknown argument: $1" >&2
        exit 2
        ;;
    esac
done

[ -n "$OUT" ] || {
    echo "usage: $0 --out DIR [--sdk DIR] [--version N]" >&2
    exit 2
}

LIBS="crt1-command.o libc.a libwasi-emulated-signal.a"
mkdir -p "$OUT/lib"

if [ -n "$SDK" ] && [ -f "$SDK/share/wasi-sysroot/lib/wasm32-wasip1/libc.a" ]; then
    for f in $LIBS; do
        cp "$SDK/share/wasi-sysroot/lib/wasm32-wasip1/$f" "$OUT/lib/"
    done
    rt=$(find "$SDK/lib/clang" -path '*wasm32-unknown-wasip1/libclang_rt.builtins.a' 2>/dev/null | head -1)
    [ -n "$rt" ] || {
        echo "no wasm32-wasip1 libclang_rt.builtins.a under $SDK/lib/clang" >&2
        exit 1
    }
    cp "$rt" "$OUT/lib/libclang_rt.builtins.a"
    SRC="local wasi-sdk at $SDK"
else
    base="https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-$VER"
    tmp=$(mktemp -d)
    trap 'rm -rf "$tmp"' EXIT
    curl -fsSL --retry 3 -o "$tmp/sr.tgz" "$base/wasi-sysroot-$VER.0.tar.gz"
    curl -fsSL --retry 3 -o "$tmp/rt.tgz" "$base/libclang_rt-$VER.0.tar.gz"
    members=
    for f in $LIBS; do
        members="$members wasi-sysroot-$VER.0/lib/wasm32-wasip1/$f"
    done
    # shellcheck disable=SC2086
    tar -xzf "$tmp/sr.tgz" -C "$tmp" $members
    tar -xzf "$tmp/rt.tgz" -C "$tmp" "libclang_rt-$VER.0/wasm32-unknown-wasip1/libclang_rt.builtins.a"
    for f in $LIBS; do
        cp "$tmp/wasi-sysroot-$VER.0/lib/wasm32-wasip1/$f" "$OUT/lib/"
    done
    cp "$tmp/libclang_rt-$VER.0/wasm32-unknown-wasip1/libclang_rt.builtins.a" "$OUT/lib/"
    SRC="wasi-sdk-$VER release ($base)"
fi

cat >"$OUT/NOTICE" <<EOF
wasm32-wasi sysroot for salam, taken from $SRC.

libc.a, crt1-command.o and libwasi-emulated-signal.a are wasi-libc
(https://github.com/WebAssembly/wasi-libc): Apache-2.0 WITH LLVM-exception,
Apache-2.0 and MIT, with musl libc code under MIT.
libclang_rt.builtins.a is LLVM compiler-rt
(https://github.com/llvm/llvm-project): Apache-2.0 WITH LLVM-exception.
EOF

ls -l "$OUT/lib"
