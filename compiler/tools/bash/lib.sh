# shellcheck shell=sh
# Shared helpers for the tools/bash/*.sh scripts, not executable on its own.

cd "$(dirname "$0")/../.." || exit 1

salam_python() {
    if   command -v python  >/dev/null 2>&1; then echo python
    elif command -v python3 >/dev/null 2>&1; then echo python3
    fi
}

salam_ensure_compiler() {
    : "${SALAM:=./salam}"
    [ -x "$SALAM" ] && return 0
    echo "building salam..." >&2
    if [ "${1:-}" = "--quiet" ]; then
        sh tools/bash/build-compiler.sh >/dev/null 2>&1
    else
        sh tools/bash/build-compiler.sh
    fi
    SALAM=./salam
}
