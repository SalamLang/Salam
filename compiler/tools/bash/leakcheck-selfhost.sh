#!/bin/sh
# Leak ratchet for the SELF-HOSTED compiler (compiler/*.salam).
#
# The C compiler is held to zero leaks (c/tools/bash/leakcheck.sh) because it
# allocates almost everything from an arena it frees on the way out. The
# self-hosted compiler cannot be held to the same bar today: Salam is manually
# memory-managed and has no arena behind its string temporaries, so every
# discarded `a + b` is a live allocation until the process exits. Demanding
# zero would mean rewriting the compiler's whole allocation discipline before
# any leak work could land at all.
#
# So this measures instead of forbidding, in the same shape as
# compiler/tools/selfhost-parity-budget.txt: a per-invocation ceiling on
# leaked allocations that CI enforces and that only ever moves down. A fix
# shows up as "budget can be lowered"; a regression fails the job.
#
# The counts are reproducible to the allocation on any one machine - the same
# binary leaks the same number of blocks every run - so the ratchet is a real
# signal rather than a threshold tuned to hide noise. They do drift by a few
# dozen BETWEEN machines, because the compiler allocates while walking
# absolute paths and no two checkouts sit at the same depth; the budgets
# therefore carry ~2% headroom and the "lower it" nag has a 5% deadband.
#
# Usage:
#   compiler/tools/bash/leakcheck-selfhost.sh <asan-selfhosted-salam> [budget-file]
#
# Env:
#   SALAM_STD   stdlib root (default: the repository's std/)
#   SALAM_CC    C compiler for the one case that links (default: gcc/clang/cc)
#
# Build the input like this, from a normal (non-ASan) salam:
#   salam build compiler/main.salam --output=salam-selfhost-asan --cc=gcc --asan
# `--asan` makes the Salam driver hand -fsanitize=address to the C compiler and
# define SALAM_MEM_DEBUG, so std/mem routes through the sanitized allocator.

set -u

ROOT=$(cd "$(dirname "$0")/../../.." && pwd)
BIN="${1:-}"
BUDGET="${2:-$ROOT/compiler/tools/selfhost-leak-budget.txt}"

[ -n "$BIN" ] || {
    echo "usage: $0 <asan-selfhosted-salam> [budget-file]" >&2
    exit 2
}
case "$BIN" in /* | [A-Za-z]:*) ;; *) BIN="$(pwd)/$BIN" ;; esac
[ -x "$BIN" ] || {
    echo "leakcheck-selfhost: '$BIN' is not executable" >&2
    exit 2
}
[ -f "$BUDGET" ] || {
    echo "leakcheck-selfhost: no budget file at '$BUDGET'" >&2
    exit 2
}

# Same guard as the C-side leakcheck: an uninstrumented binary reports a
# perfect score no matter how much it leaks, and a green job for that reason
# is worse than no job.
if ! (strings "$BIN" 2>/dev/null || cat "$BIN") |
    grep -q '__asan_init\|AddressSanitizer'; then
    echo "leakcheck-selfhost: '$BIN' is not an AddressSanitizer build" >&2
    echo "                    (rebuild it with 'salam build ... --asan')" >&2
    exit 2
fi

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

W=${TMPDIR:-/tmp}/salam-selfhost-leak.$$
rm -rf "$W"
mkdir -p "$W"
trap 'rm -rf "$W"' EXIT INT TERM
cp "$ROOT/tests/en/basics/hello.salam" "$W/hello.salam"

# exitcode=0 so a leak does not look like a crashed compiler; the SUMMARY line
# on stderr is what this reads. max_leaks is deliberately tiny: only the total
# in SUMMARY is used, and asking LSan to symbolize 100k stacks turns a
# one-second measurement into a two-minute one. Raise it (and drop
# fast_unwind_on_malloc) by hand when you want to see WHERE, not HOW MUCH.
LEAK_ASAN_OPTIONS='detect_leaks=1:exitcode=0:max_leaks=1:print_suppressions=0'

# measure <name> <args...> -> "<bytes> <allocs>"
measure() {
    name=$1
    shift
    (
        cd "$W" || exit 1
        ASAN_OPTIONS="$LEAK_ASAN_OPTIONS" "$BIN" "$@" >/dev/null 2>"$W/$name.err"
    )
    sed -n 's/.*AddressSanitizer: \([0-9]*\) byte(s) leaked in \([0-9]*\) allocation.*/\1 \2/p' \
        "$W/$name.err" | sed 1q
}

budget_for() {
    awk -v k="$1" '$1 == k { print $2; found = 1 } END { if (!found) print "" }' "$BUDGET"
}

run_case() {
    name=$1
    shift
    got=$(measure "$name" "$@")
    if [ -z "$got" ]; then
        bytes=0
        allocs=0
    else
        bytes=${got% *}
        allocs=${got#* }
    fi
    want=$(budget_for "$name")
    if [ -z "$want" ]; then
        printf '  %-14s %9s B  %7s allocs   (no budget line - add one)\n' \
            "$name" "$bytes" "$allocs"
        MISSING=$((MISSING + 1))
        return
    fi
    # The nag threshold is 5% below the budget, not "one allocation below".
    # Repeated runs of one binary on one machine are bit-identical, but the
    # count moves by a few dozen between MACHINES - the compiler allocates
    # while walking absolute paths, and /home/runner/work/Salam/Salam is not
    # the same length as anyone's checkout. Budgets therefore carry ~2%
    # headroom, and only a real improvement (past that headroom) should ask
    # to be ratcheted down.
    slack=$((want / 20))
    [ "$slack" -lt 50 ] && slack=50
    if [ "$allocs" -gt "$want" ]; then
        printf '  %-14s %9s B  %7s allocs   OVER budget %s\n' \
            "$name" "$bytes" "$allocs" "$want"
        OVER=$((OVER + 1))
    elif [ "$allocs" -lt $((want - slack)) ]; then
        printf '  %-14s %9s B  %7s allocs   well under budget %s - lower it\n' \
            "$name" "$bytes" "$allocs" "$want"
        UNDER=$((UNDER + 1))
    else
        printf '  %-14s %9s B  %7s allocs   within budget %s\n' \
            "$name" "$bytes" "$allocs" "$want"
    fi
}

OVER=0
UNDER=0
MISSING=0

echo "salam  : $BIN"
echo "budget : $BUDGET"
echo "stdlib : $SALAM_STD"
echo
echo "leaked allocations per invocation:"

run_case version version
run_case help help
run_case format format hello.salam --check
run_case inspect inspect hello.salam --emit-ast-xml
run_case exec exec hello.salam
if [ -n "${SALAM_CC:-}" ]; then
    run_case build build hello.salam --output=hello.out --cc="$SALAM_CC"
else
    echo "  build          skipped (no C compiler on this host)"
fi

echo
if [ "$MISSING" -gt 0 ]; then
    echo "leakcheck-selfhost: $MISSING case(s) have no budget line" >&2
    exit 1
fi
if [ "$OVER" -gt 0 ]; then
    echo "leakcheck-selfhost: FAILED - $OVER case(s) leak more than the budget." >&2
    echo "  Find the new leak with:" >&2
    echo "    ASAN_OPTIONS=detect_leaks=1:max_leaks=100000:fast_unwind_on_malloc=0 \\" >&2
    echo "      $BIN <the failing command> 2>&1 | less" >&2
    echo "  (rebuild the compiler with '--asan -g' first and the stacks name" >&2
    echo "   the .salam file and line that allocated)" >&2
    exit 1
fi
if [ "$UNDER" -gt 0 ]; then
    echo "leakcheck-selfhost: $UNDER case(s) improved - lower those numbers in"
    echo "  $(basename "$BUDGET") in the same change that fixed them."
fi
echo "leakcheck-selfhost: OK"
