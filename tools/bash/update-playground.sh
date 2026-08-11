#!/usr/bin/env bash
# Rebuild the Salam online playground (editor/) from the current sources.
#
# Usage: tools/bash/update-playground.sh [options]
#
#   --selfhost    Bootstrap compiler/*.salam and build the editor with the
#                 resulting self-hosted compiler instead of the C one. The
#                 Wasm bundle is unaffected either way: it is the browser
#                 compiler, emcc over c/src, and has no self-hosted form.
#   --wasm-only   Skip rebuilding the native C compiler and reuse ./salam.
#                 Ignored under --selfhost, which needs it as the seed.
#   --serve       Serve editor/ over HTTP when the build finishes.
#   --port N      Port for --serve (default 8080).
#   -h, --help    This text.

set -eu
. "$(dirname "$0")/lib.sh"
WASM_ONLY=0
SELFHOST=0
SERVE=0
PORT=8080
while [ $# -gt 0 ]; do
    case "$1" in
    --wasm-only) WASM_ONLY=1 ;;
    --selfhost) SELFHOST=1 ;;
    --serve) SERVE=1 ;;
    --port)
        shift
        PORT="${1:?--port needs a number}"
        ;;
    -h | --help)
        sed -n '2,15p' "$0"
        exit 0
        ;;
    *)
        echo "unknown option: $1 (try --help)" >&2
        exit 2
        ;;
    esac
    shift
done
if command -v taskkill >/dev/null 2>&1; then
    taskkill //F //IM salam.exe >/dev/null 2>&1 || true
fi
if [ "$WASM_ONLY" -eq 0 ] || [ "$SELFHOST" -eq 1 ]; then
    echo "==> Building native compiler (tools/bash/build-compiler.sh) ..."
    sh tools/bash/build-compiler.sh
fi
if [ "$SELFHOST" -eq 1 ]; then
    echo "==> Bootstrapping the self-hosted compiler (compiler/tools/bash/bootstrap.sh) ..."
    # Two stages, not one: stage 1 is built by the C seed and only proves the
    # seed can parse compiler/*.salam, while stage 2 is the first binary that
    # a self-hosted compiler produced, which is the one worth shipping the
    # editor from. The extra fixpoint stage belongs in the bootstrap suite,
    # not here.
    sh compiler/tools/bash/bootstrap.sh ./salam build/selfhost --stages 2
    SALAM="$(pwd)/build/selfhost/salam-stage2"
    [ -x "$SALAM" ] || SALAM="$SALAM.exe"
    [ -x "$SALAM" ] || {
        echo "error: bootstrap left no stage2 compiler at build/selfhost" >&2
        exit 1
    }
    export SALAM
    echo "==> Editor sources will be compiled by $SALAM"
fi
echo "==> Building WebAssembly bundle (tools/bash/build-wasm.sh) ..."
sh tools/bash/build-wasm.sh
echo "==> Editor bundle:"
for f in editor/salam-wa.js editor/salam-wa.wasm editor/salam-wa.data; do
    if [ -f "$f" ]; then
        size=$(wc -c <"$f" | tr -d ' ')
        printf '    %-22s %s bytes\n' "$f" "$size"
    else
        echo "    MISSING: $f" >&2
    fi
done
echo "==> Exported entry points:"
for sym in _salam_web_run_app _salam_web_compile_js _salam_web_build_layout _salam_web_emit _salam_web_syntax_ok; do
    if grep -q "$sym" editor/salam-wa.js 2>/dev/null; then
        echo "    ok   $sym"
    else echo "    WARN missing $sym" >&2; fi
done
echo "==> Done. Reload the editor to pick up the new build."
if [ "$SERVE" -eq 1 ]; then
    exec sh tools/bash/editor-serve.sh "$PORT"
fi
