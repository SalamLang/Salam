#!/usr/bin/env bash
# Rebuild the Salam online playground (web/) from the current sources.

set -eu
. "$(dirname "$0")/lib.sh"
WASM_ONLY=0
SERVE=0
PORT=8080
while [ $# -gt 0 ]; do
    case "$1" in
        --wasm-only) WASM_ONLY=1 ;;
        --serve)     SERVE=1 ;;
        --port)      shift; PORT="${1:?--port needs a number}" ;;
        -h|--help)   sed -n '2,19p' "$0"; exit 0 ;;
        *) echo "unknown option: $1 (try --help)" >&2; exit 2 ;;
    esac
    shift
done
if command -v taskkill >/dev/null 2>&1; then
    taskkill //F //IM salamc.exe >/dev/null 2>&1 || true
fi
if [ "$WASM_ONLY" -eq 0 ]; then
    echo "==> Building native compiler (tools/build-compiler.sh) ..."
    sh tools/build-compiler.sh
fi
echo "==> Building WebAssembly bundle (tools/build-wasm.sh) ..."
sh tools/build-wasm.sh
echo "==> Editor bundle:"
for f in web/salam-wa.js web/salam-wa.wasm web/salam-wa.data; do
    if [ -f "$f" ]; then
        size=$(wc -c < "$f" | tr -d ' ')
        printf '    %-22s %s bytes\n' "$f" "$size"
    else
        echo "    MISSING: $f" >&2
    fi
done
echo "==> Exported entry points:"
for sym in _salam_web_run_app _salam_web_build_layout _salam_web_emit; do
    if grep -q "$sym" web/salam-wa.js 2>/dev/null; then echo "    ok   $sym"
    else echo "    WARN missing $sym" >&2; fi
done
echo "==> Done. Reload the editor to pick up the new build."
if [ "$SERVE" -eq 1 ]; then
    exec sh tools/editor-serve.sh "$PORT"
fi
