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

Each line is one of:

    AGREE            output matched the .out file
    DIVERGE          it ran and printed something else
    DIVERGE-TIMEOUT  it did not finish; output is whatever it flushed
    SKIP-NOEXP       no .out file to compare against
    SKIP-FAIL        the backend could not build or emit it at all

Output is compared exactly the way run-tests.sh compares it, so that a pass
here means the same thing a pass there does. A non-zero exit is not by itself
a failure, since the runner does not treat it as one either.

A DIVERGE is not automatically a bug: the JS backend has no 64-bit integers,
sockets, sqlite or FFI, and the interpreter refuses variadic externs and is
far slower, so check the cause before reporting one.

Every DIVERGE the default sections produce under 'exec' as of 2026-10-05 was
checked, and none of them is an interpreter bug:

    interop/ffi_libm is a variadic extern, which it refuses with a clear
    error, and interop/redis_demo (en and fa) wants a live redis server.

    general/websocket_wss_loopback fails because std/tls enforces a 15s
    handshake read deadline (HANDSHAKE_READ_TIMEOUT_MS in std/tls/record)
    and the interpreted crypto needs about 75s a side, so the peer tears the
    connection down mid-handshake. The ssl/ section, which is not in the
    default set, fails the same way, and stdlib/tls_selftest runs the same
    crypto into the sweep timeout.

    stdlib/nn_transformer_demo is interpreted matmul meeting that timeout.

    stdlib/ssh_selftest exhausts memory in interpreted SSH crypto, so give
    it a 'ulimit -v' rather than letting the OOM killer choose a victim.

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

# Must match tools/bash/run-tests.sh: the expectation files are named after
# its spellings ('mac', 'x64'), not uname's.
case "$(uname -s 2>/dev/null)" in
Linux) HOST_OS=linux ;;
Darwin) HOST_OS=mac ;;
MINGW* | MSYS* | CYGWIN*) HOST_OS=windows ;;
*) HOST_OS="" ;;
esac
[ "${OS:-}" = "Windows_NT" ] && HOST_OS=windows
case "$(uname -m 2>/dev/null)" in
x86_64 | amd64) HOST_ARCH=x64 ;;
aarch64 | arm64) HOST_ARCH=arm64 ;;
i386 | i486 | i586 | i686 | x86) HOST_ARCH=x86 ;;
armv6l | armv7l | armv7 | arm) HOST_ARCH=arm ;;
*) HOST_ARCH="" ;;
esac

pick_expect() {
    if [ -n "$HOST_OS" ] && [ -n "$HOST_ARCH" ] && [ -f "$1.$HOST_OS.$HOST_ARCH.out" ]; then
        printf '%s\n' "$1.$HOST_OS.$HOST_ARCH.out"
    elif [ -n "$HOST_OS" ] && [ -f "$1.$HOST_OS.out" ]; then
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
            rc=0
            case "$BACKEND" in
            exec)
                # shellcheck disable=SC2086
                timeout "${SALAM_SWEEP_TIMEOUT:-150}" "$SALAM_BIN" exec "$ROOT/$f" $defs \
                    --no-color --log-level=error --lang="$lang" </dev/null >"$WORK/run.out" 2>&1
                rc=$?
                got=$(tr -d '\r' <"$WORK/run.out")
                ;;
            js)
                # shellcheck disable=SC2086
                if (cd "$WORK" && timeout "${SALAM_SWEEP_TIMEOUT:-150}" "$SALAM_BIN" js "$ROOT/$f" $defs \
                    --output=sweep.js --no-color --log-level=error --lang="$lang") >/dev/null 2>&1; then
                    (cd "$WORK" && timeout 90 node sweep.js </dev/null) >"$WORK/run.out" 2>&1
                    rc=$?
                    got=$(tr -d '\r' <"$WORK/run.out")
                else
                    ok=0
                fi
                ;;
            llvm)
                # shellcheck disable=SC2086
                if (cd "$WORK" && timeout "${SALAM_SWEEP_TIMEOUT:-200}" "$SALAM_BIN" build "$ROOT/$f" $defs \
                    --backend=llvm --output=sweep.exe --no-color --log-level=error --lang="$lang") >/dev/null 2>&1 &&
                    [ -x "$WORK/sweep.exe" ]; then
                    (cd "$WORK" && timeout 90 ./sweep.exe </dev/null) >"$WORK/run.out" 2>&1
                    rc=$?
                    got=$(tr -d '\r' <"$WORK/run.out")
                else
                    ok=0
                fi
                rm -f "$WORK/sweep.exe"
                ;;
            esac
            if [ "$ok" -eq 0 ]; then
                echo "SKIP-FAIL $f"
                skipped=$((skipped + 1))
            elif [ "$rc" -eq 124 ]; then
                echo "DIVERGE-TIMEOUT $f"
                diverge=$((diverge + 1))
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
