# Shared helpers for the tools/*.sh scripts — not executable on its own.

cd "$(dirname "$0")/.." || exit 1
SALAM_ROOT="$(pwd)"
salam_python() {
    if   command -v python  >/dev/null 2>&1; then echo python
    elif command -v python3 >/dev/null 2>&1; then echo python3
    fi
}
salam_ensure_compiler() {
    : "${SALAMC:=./salamc}"
    [ -x "$SALAMC" ] && return 0
    echo "building salamc..." >&2
    if [ "${1:-}" = "--quiet" ]; then
        sh tools/build-compiler.sh >/dev/null 2>&1
    else
        sh tools/build-compiler.sh
    fi
    SALAMC=./salamc
}
