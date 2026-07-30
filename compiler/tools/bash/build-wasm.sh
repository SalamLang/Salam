#!/bin/sh
# Build the Salam compiler to WebAssembly for the online playground.
#
# Self-hosted: compiler/sal_web.salam (not hand-written C) defines the four
# functions the playground's JS calls via Module.cwrap - RunApp, BuildLayout,
# Emit, Version. `salam obj` self-hosts them into salam_mod_*.c the same way
# it does for any Salam build; --export=Fn:CName keeps each one alive despite
# dead-code elimination and emits it under the literal C name emcc's
# EXPORTED_FUNCTIONS list below expects (see compiler/codegen.salam's
# AddExportOverride / cg_export_override - there is no C source here anymore).

set -e
. "$(dirname "$0")/lib.sh"
salam_ensure_compiler
rm -f salam_mod_*.c salam_mod_*.h salam_mod_*.o
"$SALAM" run compiler/tools/salam/gen-examples.salam
EMCC="${EMCC:-emcc}"
if ! command -v "$EMCC" >/dev/null 2>&1; then
    EMSDK_DIR="${SALAM_EMSDK:-C:/emsdk-wasm}"
    EMCC_LOCAL="$EMSDK_DIR/install/emscripten/emcc.exe"
    [ -x "$EMCC_LOCAL" ] || EMCC_LOCAL="$EMSDK_DIR/install/emscripten/emcc"
    if [ -e "$EMCC_LOCAL" ]; then
        EMCC="$EMCC_LOCAL"
        export PATH="$EMSDK_DIR/install/emscripten:$PATH"
        export EM_CACHE="${EM_CACHE:-$EMSDK_DIR/cache}"
        if [ -z "$EM_CONFIG" ] && [ ! -f "$EMSDK_DIR/.emscripten" ]; then
            NODE_EXE="$(command -v node)"
            {
                echo "LLVM_ROOT = '$EMSDK_DIR/install/bin'"
                echo "BINARYEN_ROOT = '$EMSDK_DIR/install'"
                echo "EMSCRIPTEN_ROOT = '$EMSDK_DIR/install/emscripten'"
                echo "NODE_JS = '$(cygpath -m "$NODE_EXE" 2>/dev/null || echo "$NODE_EXE")'"
                echo "CACHE = '$EMSDK_DIR/cache'"
                echo "COMPILER_ENGINE = NODE_JS"
                echo "JS_ENGINES = [NODE_JS]"
            } >"$EMSDK_DIR/.emscripten"
        fi
        export EM_CONFIG="${EM_CONFIG:-$EMSDK_DIR/.emscripten}"
    fi
fi
command -v "$EMCC" >/dev/null 2>&1 || [ -e "$EMCC" ] || {
    echo "error: emcc not found." >&2
    echo "       Install the Emscripten SDK and source emsdk_env.sh, or point" >&2
    echo "       \$SALAM_EMSDK at an unpacked emscripten-releases bundle (default C:/emsdk-wasm)." >&2
    echo "       https://emscripten.org/docs/getting_started/downloads.html" >&2
    if grep -qi microsoft /proc/version 2>/dev/null; then
        echo "" >&2
        echo "       You are in WSL. If 'emsdk install' can't download, run this step" >&2
        echo "       from Windows (Git Bash/PowerShell) with the existing C:/emsdk-wasm" >&2
        echo "       bundle:  sh compiler/tools/bash/build-wasm.sh" >&2
    fi
    exit 1
}

OUT_DIR="editor"
mkdir -p "$OUT_DIR"

echo "==> self-hosting compiler/sal_web.salam -> salam_mod_*.c (kept for emcc)"
SALAM_STD="$(pwd)" "$SALAM" obj compiler/sal_web.salam --keep-c \
    --export=RunApp:salam_web_run_app \
    --export=BuildLayout:salam_web_build_layout \
    --export=Emit:salam_web_emit \
    --export=Version:salam_web_version
rm -f salam_mod_*.o

# shellcheck disable=SC2046
"$EMCC" -O2 -I. $(ls salam_mod_*.c) \
    -o "$OUT_DIR/salam-wa.js" \
    --preload-file std@/std \
    -s MODULARIZE=0 \
    -s ENVIRONMENT=web,worker,node \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s INITIAL_MEMORY=33554432 \
    -s STACK_SIZE=4194304 \
    -s EXIT_RUNTIME=0 \
    -s IGNORE_MISSING_MAIN=1 \
    -s FILESYSTEM=1 \
    -s EXPORTED_FUNCTIONS="['_salam_web_run_app','_salam_web_build_layout','_salam_web_emit','_salam_web_version','_malloc','_free']" \
    -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap','UTF8ToString','stringToUTF8','lengthBytesUTF8','FS']"
echo "built $OUT_DIR/salam-wa.js (+ .wasm, .data)"
rm -f salam_mod_*.c salam_mod_*.h
"$SALAM" web "$OUT_DIR/src/page.salam" --output="$OUT_DIR/index.html"
echo "built $OUT_DIR/index.html"
