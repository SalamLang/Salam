#!/bin/sh
# Build the Salam compiler to WebAssembly for the online playground.

set -e
. "$(dirname "$0")/lib.sh"
salam_ensure_compiler
rm -rf .salam-build
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
        echo "       bundle:  sh tools/build-wasm.sh" >&2
    fi
    exit 1
}

OUT_DIR="../editor"
mkdir -p "$OUT_DIR"

# --- stdlib preload image -------------------------------------------------
# The browser compiler can only ever *import* .salam sources, so stage a
# preload image holding exactly those. std/doc.html alone is ~2 MB of
# generated documentation and std/opencv/native/* is C++/CMake consumed only
# by native builds; neither is reachable from an `import`, so preloading the
# raw std/ tree shipped megabytes the playground could never open.
# Then minify the staged copy: `format --minify` strips comments, indentation
# and blank lines while leaving the token stream identical (verified across
# all stdlib files), worth another ~500 KB the browser would download and
# then discard, since the lexer drops that trivia anyway.
STD_MIN="$(pwd)/build/std-min"
rm -rf "$STD_MIN"
mkdir -p "$STD_MIN"
(
    cd ../std
    find . -name '*.salam' | while IFS= read -r f; do
        mkdir -p "$STD_MIN/$(dirname "$f")"
        cp "$f" "$STD_MIN/$f"
    done
)
"$SALAM" format --minify -r "$STD_MIN" >/dev/null
echo "staged minified stdlib preload image at $STD_MIN"
SRC_DIRS="core source logger xml condcomp token langpack i18n lexer ast parser
        diag semantic interp layout minify codegen llvm jsgen web"
SRCS=""
for d in $SRC_DIRS; do SRCS="$SRCS src/$d/*.c"; done
# js_build.c only, not all of src/driver: it is the bundler that turns jsgen's
# per-module output into one runnable program (prelude, globals, entry call),
# which is exactly what salam_web_compile_js needs. The rest of the driver
# shells out to a C toolchain and has no meaning in the browser.
SRCS="$SRCS src/driver/js_build.c"
# shellcheck disable=SC2086
"$EMCC" -O2 -Isrc $SRCS \
    -o "$OUT_DIR/salam-wa.js" \
    --preload-file "$STD_MIN"@/std \
    -s MODULARIZE=0 \
    -s ENVIRONMENT=web,worker,node \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s INITIAL_MEMORY=33554432 \
    -s STACK_SIZE=4194304 \
    -s EXIT_RUNTIME=0 \
    -s IGNORE_MISSING_MAIN=1 \
    -s FILESYSTEM=1 \
    -s EXPORTED_FUNCTIONS="['_salam_web_run_app','_salam_web_compile_js','_salam_web_build_layout','_salam_web_emit','_salam_web_syntax_ok','_salam_web_version','_malloc','_free']" \
    -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap','UTF8ToString','stringToUTF8','lengthBytesUTF8','FS']"
echo "built $OUT_DIR/salam-wa.js (+ .wasm, .data)"
"$SALAM" web "$OUT_DIR/page.salam" --output="$OUT_DIR/index.html"
echo "built $OUT_DIR/index.html"
