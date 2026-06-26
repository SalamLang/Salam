#!/bin/sh
# Build the Salam compiler to WebAssembly for the online playground.

set -e
. "$(dirname "$0")/lib.sh"
salam_ensure_compiler
rm -f salam_mod_*.c salam_mod_*.h
"$SALAM" run tools/salam/gen-examples.salam
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
            } > "$EMSDK_DIR/.emscripten"
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
        echo "       bundle:  sh tools/build-wasm.sh" >&2
    fi
    exit 1
}

OUT_DIR="../editor"
mkdir -p "$OUT_DIR"
"$SALAM" run tools/salam/gen-editor-keywords.salam || echo "warning: keyword generation failed; using existing ../editor/keywords.js" >&2
SRC_DIRS="core source logger xml preproc token langpack i18n lexer ast parser
          diag semantic interp layout codegen llvm web"
SRCS=""
for d in $SRC_DIRS; do SRCS="$SRCS src/$d/*.c"; done
"$EMCC" -O2 -Isrc $SRCS \
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
