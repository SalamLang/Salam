#!/bin/sh

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

OUT_DIR="editor"
mkdir -p "$OUT_DIR"

# The bundle is stamped with the Salam version. A release therefore asks for
# file names that no browser or service worker cache can still hold, so a new
# version is never served from an older one, and the untouched files can be
# cached for as long as the site likes.
VERSION=$(tr -d ' \t\r\n' <VERSION)
[ -n "$VERSION" ] || {
    echo "error: cannot read VERSION (run this from the repository root)" >&2
    exit 1
}
sed "s|^pub mut VERSION := \".*\"$|pub mut VERSION := \"$VERSION\"|" \
    "$OUT_DIR/build_info.salam" >"$OUT_DIR/build_info.salam.tmp"
mv "$OUT_DIR/build_info.salam.tmp" "$OUT_DIR/build_info.salam"
sed "s|^const SW_VERSION = \".*\";$|const SW_VERSION = \"$VERSION\";|" \
    "$OUT_DIR/sw.js" >"$OUT_DIR/sw.js.tmp"
mv "$OUT_DIR/sw.js.tmp" "$OUT_DIR/sw.js"
grep -q "^pub mut VERSION := \"$VERSION\"$" "$OUT_DIR/build_info.salam" || {
    echo "error: could not stamp editor/build_info.salam" >&2
    exit 1
}
BUNDLE="$OUT_DIR/salam-wa-$VERSION"

STD_MIN="$(pwd)/.wasm-build/std-min"
rm -rf "$STD_MIN"
mkdir -p "$STD_MIN"
(
    cd std
    find . -name '*.salam' | while IFS= read -r f; do
        mkdir -p "$STD_MIN/$(dirname "$f")"
        cp "$f" "$STD_MIN/$f"
    done
)
"$SALAM" format --minify -r "$STD_MIN" >/dev/null
echo "staged minified stdlib preload image at $STD_MIN"

rm -rf .salam-build
"$SALAM" build --backend=c --emit-c --target=wasm32-unknown-emscripten \
    compiler/main.salam --output=.wasm-build/host-salam --log-level=warn
SRCS=$(find .salam-build -name '*.c' | sort | tr '\n' ' ')
[ -n "$SRCS" ] || {
    echo "no generated C in .salam-build; the compiler build produced nothing" >&2
    exit 1
}

# shellcheck disable=SC2086
"$EMCC" -O2 -I.salam-build $SRCS \
    -o "$BUNDLE.js" \
    --preload-file "$STD_MIN"@/std \
    -s MODULARIZE=0 \
    -s ENVIRONMENT=web,worker,node \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s INITIAL_MEMORY=33554432 \
    -s STACK_SIZE=16777216 \
    -s EXIT_RUNTIME=0 \
    -s IGNORE_MISSING_MAIN=1 \
    -s FILESYSTEM=1 \
    -s EXPORTED_FUNCTIONS="['_salam_web_run_app','_salam_web_build_layout','_salam_web_emit','_salam_web_syntax_ok','_salam_web_last_failed','_salam_web_version','_malloc','_free']" \
    -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap','UTF8ToString','stringToUTF8','lengthBytesUTF8','FS']"
echo "built $BUNDLE.js (+ .wasm, .data)"
"$SALAM" web "$OUT_DIR/page.salam" --output="$OUT_DIR/index.html"
echo "built $OUT_DIR/index.html"
