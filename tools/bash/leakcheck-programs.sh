#!/bin/sh
# Leak ratchet for COMPILED SALAM PROGRAMS (the stdlib demos), as opposed to
# the compiler that builds them.
#
# tools/bash/leakcheck.sh and compiler/tools/bash/leakcheck-selfhost.sh both
# measure `salam` itself. Nothing measured the programs it emits, and that is
# where a library leak actually hurts: a demo runs for a second, but the same
# std/ code inside a training loop or a server runs for hours. Sweeping the
# demos this way in 2026-08 found a closure environment leaked per loop
# iteration (a decision tree dropped 460 KB fitting one forest), the compare
# asserts in std/testing formatting their message on every PASS, and unowned
# strings in csv, data, gguf, onnx, llm and nn.
#
# Each case builds one test program with `--asan` and runs it, then reads
# LeakSanitizer's own count of blocks still unreachable at exit. The budget
# file gives a ceiling per program; the script FAILS when a program leaks
# more and WARNS when it leaks less, exactly like the self-host ratchet.
# Most lines are 0 and must stay 0.
#
# Usage:
#   sh tools/bash/leakcheck-programs.sh <salam> [budget-file]
#
# Env:
#   SALAM_STD   stdlib root (default: the repository's std/)
#   SALAM_CC    C compiler to build with (default: gcc, then clang, then cc)
#   KEEP        set to 1 to keep the build directories for inspection
#
# `<salam>` is an ORDINARY compiler, not an ASan one: `--asan` is passed to
# the build, so it is the program under test that is instrumented.

set -u

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BIN="${1:-}"
BUDGET="${2:-$ROOT/tools/program-leak-budget.txt}"

[ -n "$BIN" ] || {
    echo "usage: $0 <salam> [budget-file]" >&2
    exit 2
}
case "$BIN" in /* | [A-Za-z]:*) ;; *) BIN="$(pwd)/$BIN" ;; esac
[ -x "$BIN" ] || {
    echo "leakcheck-programs: '$BIN' is not executable" >&2
    exit 2
}
[ -f "$BUDGET" ] || {
    echo "leakcheck-programs: no budget file at '$BUDGET'" >&2
    exit 2
}

: "${SALAM_STD:=$ROOT/std}"
export SALAM_STD
if [ -z "${SALAM_CC:-}" ]; then
    for c in gcc clang cc; do
        command -v "$c" >/dev/null 2>&1 && {
            SALAM_CC=$c
            break
        }
    done
fi
[ -n "${SALAM_CC:-}" ] || {
    echo "leakcheck-programs: no C compiler on this host" >&2
    exit 2
}

W=${TMPDIR:-/tmp}/salam-program-leak.$$
rm -rf "$W"
mkdir -p "$W"
[ "${KEEP:-0}" = "1" ] || trap 'rm -rf "$W"' EXIT INT TERM

# exitcode=0 so a leak does not look like a crashed program; the SUMMARY line
# on stderr is what this reads. max_leaks=1 keeps LSan from symbolizing every
# stack, which turns a one-second measurement into a two-minute one - raise it
# by hand (and drop fast_unwind_on_malloc) when you want to see WHERE.
LEAK_ASAN_OPTIONS='detect_leaks=1:exitcode=0:max_leaks=1:print_suppressions=0'

budget_for() {
    awk -v k="$1" '$1 == k { print $2; found = 1 } END { if (!found) print "" }' "$BUDGET"
}

OVER=0
UNDER=0
MISSING=0
BROKE=0

# run_case <demo-name>  (tests/en/stdlib/<demo-name>.salam)
run_case() {
    name=$1
    src="$ROOT/tests/en/stdlib/$name.salam"
    if [ ! -f "$src" ]; then
        printf '  %-24s missing source %s\n' "$name" "$src"
        BROKE=$((BROKE + 1))
        return
    fi
    # Its own directory: .salam-build lands in the working directory, and two
    # builds sharing one clobber each other's objects.
    d="$W/$name"
    mkdir -p "$d"
    if ! (cd "$d" && "$BIN" build "$src" --output="$d/prog" --asan \
        --cc="$SALAM_CC" --log-level=error >"$d/build.log" 2>&1); then
        printf '  %-24s BUILD FAILED (see %s)\n' "$name" "$d/build.log"
        BROKE=$((BROKE + 1))
        return
    fi
    (cd "$d" && ASAN_OPTIONS="$LEAK_ASAN_OPTIONS" "$d/prog" \
        >"$d/run.out" 2>"$d/run.err")
    # A demo that fails its own assertions is not a leak measurement.
    if ! grep -q '0 failed' "$d/run.out" 2>/dev/null; then
        printf '  %-24s TEST FAILED (see %s)\n' "$name" "$d/run.out"
        BROKE=$((BROKE + 1))
        return
    fi
    got=$(sed -n 's/.*AddressSanitizer: \([0-9]*\) byte(s) leaked in \([0-9]*\) allocation.*/\1 \2/p' \
        "$d/run.err" | sed 1q)
    if [ -z "$got" ]; then
        bytes=0
        allocs=0
    else
        bytes=${got% *}
        allocs=${got#* }
    fi
    want=$(budget_for "$name")
    if [ -z "$want" ]; then
        printf '  %-24s %9s B  %6s allocs   (no budget line - add one)\n' \
            "$name" "$bytes" "$allocs"
        MISSING=$((MISSING + 1))
        return
    fi
    if [ "$allocs" -gt "$want" ]; then
        printf '  %-24s %9s B  %6s allocs   OVER budget %s\n' \
            "$name" "$bytes" "$allocs" "$want"
        OVER=$((OVER + 1))
    elif [ "$allocs" -lt "$want" ]; then
        printf '  %-24s %9s B  %6s allocs   under budget %s - lower it\n' \
            "$name" "$bytes" "$allocs" "$want"
        UNDER=$((UNDER + 1))
    else
        printf '  %-24s %9s B  %6s allocs   at budget %s\n' \
            "$name" "$bytes" "$allocs" "$want"
    fi
}

echo "salam  : $BIN"
echo "budget : $BUDGET"
echo "stdlib : $SALAM_STD"
echo "cc     : $SALAM_CC"
echo
echo "leaked allocations per program:"

# Every case named in the budget file, in the order it appears there, so the
# list lives in one place.
CASES=$(awk '$1 !~ /^#/ && NF == 2 { print $1 }' "$BUDGET")
for c in $CASES; do
    run_case "$c"
done

echo
if [ "$BROKE" -gt 0 ]; then
    echo "leakcheck-programs: $BROKE case(s) did not build or did not pass" >&2
    exit 1
fi
if [ "$MISSING" -gt 0 ]; then
    echo "leakcheck-programs: $MISSING case(s) have no budget line" >&2
    exit 1
fi
if [ "$OVER" -gt 0 ]; then
    echo "leakcheck-programs: FAILED - $OVER program(s) leak more than the budget." >&2
    echo "  Find the new leak with:" >&2
    echo "    salam build tests/en/stdlib/<case>.salam --output=/tmp/p --asan --cc=gcc" >&2
    echo "    ASAN_OPTIONS=detect_leaks=1:max_leaks=100000:fast_unwind_on_malloc=0 /tmp/p" >&2
    echo "  then group the report by its deepest _Salam_* frame." >&2
    exit 1
fi
if [ "$UNDER" -gt 0 ]; then
    echo "leakcheck-programs: $UNDER case(s) improved - lower those numbers in"
    echo "  $(basename "$BUDGET") in the same change that fixed them."
fi
echo "leakcheck-programs: OK"
