# shellcheck shell=sh
# Shared helpers for the tools/bash/*.sh scripts, not executable on its own.

# Anchor at the REPO ROOT (self-host restructure: std/ and tests/ live here
# as siblings of compiler/, not inside it). This file is at
# compiler/tools/bash/lib.sh, so three levels up is the repo root.
cd "$(dirname "$0")/../../.." || exit 1

salam_python() {
    if command -v python >/dev/null 2>&1; then
        echo python
    elif command -v python3 >/dev/null 2>&1; then
        echo python3
    fi
}

# Ensures compiler/salam exists and is runnable, (re)building it from source
# via self-hosting (compiler/salam building compiler/main.salam) if missing.
# There is no C source anymore — compiler/salam IS the tracked bootstrap
# binary, so this only rebuilds when someone explicitly deletes it or wants
# a fresh self-hosted build reflecting current .salam source changes.
salam_ensure_compiler() {
    : "${SALAM:=./compiler/salam}"
    [ -x "$SALAM" ] && return 0
    echo "building salam (self-hosted, from compiler/main.salam)..." >&2
    if [ "${1:-}" = "--quiet" ]; then
        sh compiler/tools/bash/build-compiler.sh >/dev/null 2>&1
    else
        sh compiler/tools/bash/build-compiler.sh
    fi
    SALAM=./compiler/salam
}
