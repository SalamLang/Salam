#!/bin/sh

set -u

SALAM_BIN="${1:-}"
BACKEND="${2:-}"
[ -n "$SALAM_BIN" ] && [ -n "$BACKEND" ] || {
    cat >&2 <<'USAGE'
usage: backend-sweep.sh <salam-binary> <exec|js|llvm> [sections...]

Runs test programs on a backend other than the C one and compares their
output with the .out file the C backend is held to. run-tests.sh gives almost
every section a C build and only runs the interpreter for exec/ and the JS
backend for js/, so most tests never execute anywhere else; this is how to
cover that gap.

Default sections: general features basics types switch match data
    editor-selected interop stdlib

Each line is AGREE, DIVERGE, SKIP-NOEXP (no .out) or SKIP-FAIL (the backend
could not build or run it). A DIVERGE is not automatically a bug: the JS
backend has no 64-bit integers, sockets, sqlite or FFI, and the interpreter
refuses variadic externs and is far slower, so check the cause before
reporting one.
USAGE
    exit 2
}
case "$SALAM_BIN" in /* | [A-Za-z]:*) ;; *) SALAM_BIN="$(pwd)/$SALAM_BIN" ;; esac
[ -x "$SALAM_BIN" ] || {
    echo "backend-sweep: '$SALAM_BIN' is not executable" >&2
    exit 2
}
case "$BACKEND" in
exec | js | llvm) ;;
*)
    echo "backend-sweep: backend must be exec, js or llvm" >&2
    exit 2
    ;;
esac

ROOT=$(pwd)
[ -d "$ROOT/tests" ] || {
    echo "backend-sweep: run this from the repository root" >&2
    exit 2
}
if [ -z "${SALAM_STD:-}" ] && [ -d "$ROOT/std" ]; then
    SALAM_STD="$ROOT/std"
    export SALAM_STD
fi

shift 2
SECTIONS="$*"
[ -n "$SECTIONS" ] ||
    SECTIONS="general features basics types switch match data editor-selected interop stdlib"

WORK=$(mktemp -d "${TMPDIR:-/tmp}/salam-sweep.XXXXXX") || exit 2
trap 'rm -rf "$WORK"' EXIT INT TERM

HOST_OS=linux
case "$(uname -s 2>/dev/null)" in
Darwin) HOST_OS=macos ;;
MINGW* | MSYS* | CYGWIN*) HOST_OS=windows ;;
esac
HOST_ARCH=$(uname -m 2>/dev/null)
case "$HOST_ARCH" in
x86_64 | amd64) HOST_ARCH=x86_64 ;;
aarch64 | arm64) HOST_ARCH=arm64 ;;
*) HOST_ARCH="" ;;
esac

pick_expect() {
    if [ -n "$HOST_ARCH" ] && [ -f "$1.$HOST_OS.$HOST_ARCH.out" ]; then
        printf '%s\n' "$1.$HOST_OS.$HOST_ARCH.out"
    elif [ -f "$1.$HOST_OS.out" ]; then
        printf '%s\n' "$1.$HOST_OS.out"
    else
        printf '%s\n' "$1.out"
    fi
}

agree=0
diverge=0
skipped=0

for section in $SECTIONS; do
    for lang in en fa; do
        dir="tests/$lang/$section"
        [ -d "$dir" ] || continue
        for f in "$dir"/*.salam; do
            [ -e "$f" ] || continue
            name=$(basename "$f" .salam)
            case "$name" in _*) continue ;; esac
            exp=$(pick_expect "$dir/$name")
            if [ ! -f "$exp" ]; then
                echo "SKIP-NOEXP $f"
                skipped=$((skipped + 1))
                continue
            fi
            defs=$(grep -o 'DEFINE: [A-Za-z0-9_]*' "$f" | sed 's/DEFINE: /-D/' | tr '\n' ' ')
            defs="$defs$(grep -o 'CONST: [!-~]*' "$f" | sed 's/CONST: /-d/' | tr '\n' ' ')"
            want=$(tr -d '\r' <"$exp")
            got=""
            ok=1
            case "$BACKEND" in
            exec)
                # shellcheck disable=SC2086
                got=$(timeout "${SALAM_SWEEP_TIMEOUT:-150}" "$SALAM_BIN" exec "$ROOT/$f" $defs \
                    --no-color --log-level=error --lang="$lang" </dev/null 2>&1 | tr -d '\r') || ok=1
                ;;
            js)
                # shellcheck disable=SC2086
                if (cd "$WORK" && timeout "${SALAM_SWEEP_TIMEOUT:-150}" "$SALAM_BIN" js "$ROOT/$f" $defs \
                    --output=sweep.js --no-color --log-level=error --lang="$lang") >/dev/null 2>&1; then
                    got=$(cd "$WORK" && timeout 90 node sweep.js </dev/null 2>&1 | tr -d '\r')
                else
                    ok=0
                fi
                ;;
            llvm)
                # shellcheck disable=SC2086
                if (cd "$WORK" && timeout "${SALAM_SWEEP_TIMEOUT:-200}" "$SALAM_BIN" build "$ROOT/$f" $defs \
                    --backend=llvm --output=sweep.exe --no-color --log-level=error --lang="$lang") >/dev/null 2>&1 &&
                    [ -x "$WORK/sweep.exe" ]; then
                    got=$(cd "$WORK" && timeout 90 ./sweep.exe </dev/null 2>&1 | tr -d '\r')
                else
                    ok=0
                fi
                rm -f "$WORK/sweep.exe"
                ;;
            esac
            if [ "$ok" -eq 0 ]; then
                echo "SKIP-FAIL $f"
                skipped=$((skipped + 1))
            elif [ "$got" = "$want" ]; then
                echo "AGREE $f"
                agree=$((agree + 1))
            else
                echo "DIVERGE $f"
                diverge=$((diverge + 1))
            fi
        done
    done
done

echo "----------------------------------------"
echo "RESULT ($BACKEND): $agree agree, $diverge diverge, $skipped skipped"
[ "$diverge" -eq 0 ]
