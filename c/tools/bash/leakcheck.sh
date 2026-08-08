#!/bin/sh
# Run the whole test corpus through an AddressSanitizer/LeakSanitizer build
# of salam and fail if the compiler leaked anything.
#
# Usage:
#   sh c/tools/bash/leakcheck.sh [-j N] [--no-suite] [--no-sweep]
#                               [--reports=DIR] [--allow=N] [section ...]
#
# Env:
#   SALAM        salam binary to test. MUST be built with -fsanitize=address;
#                the script refuses to run otherwise, because an
#                uninstrumented binary reports zero leaks no matter what.
#                Default: build/asan/salam, which `--build` produces.
#   SALAM_STD    stdlib root (defaults to ./std, as run-tests.sh does)
#   LSAN_SUPP    suppression file (default: c/tools/lsan.supp)
#   NPROC        parallel workers, same meaning as in run-tests.sh
#
# Options:
#   --build      build the ASan salam first (make -C c ... into build/asan)
#   --no-suite   skip run-tests.sh, only run the extra-surface sweep
#   --no-sweep   skip the extra-surface sweep, only run run-tests.sh
#   --reports=D  where to collect the raw LSan reports (default: a temp dir)
#   --allow=N    tolerate up to N leaking processes instead of 0. Only for
#                ratcheting down a known backlog - the C compiler is at 0 and
#                must stay there.
#   section ...  forwarded to run-tests.sh (exec, errors, llvm, ...)
#
# Why this exists as a script and not just an ASAN_OPTIONS line in CI: a leak
# is reported by the *child* process that leaked, at its exit, long after the
# test that spawned it has been graded PASS. Writing every report to its own
# file under one directory (log_path) and counting the files afterwards is
# the only way to see them all - and `exitcode=0` keeps a leak from turning
# into a spurious test failure that hides which test actually leaked.

set -u

. "$(dirname "$0")/lib.sh"

NPROC_ARG=""
DO_SUITE=1
DO_SWEEP=1
DO_BUILD=0
REPORTS=""
ALLOW=0
SECTIONS=""

while [ $# -gt 0 ]; do
    case "$1" in
    -j)
        NPROC_ARG="-j $2"
        shift 2
        ;;
    -j* | --jobs=*)
        NPROC_ARG="$1"
        shift
        ;;
    --build)
        DO_BUILD=1
        shift
        ;;
    --no-suite)
        DO_SUITE=0
        shift
        ;;
    --no-sweep)
        DO_SWEEP=0
        shift
        ;;
    --reports=*)
        REPORTS="${1#--reports=}"
        shift
        ;;
    --allow=*)
        ALLOW="${1#--allow=}"
        shift
        ;;
    -h | --help)
        sed -n '2,36p' "$0"
        exit 0
        ;;
    -*)
        echo "leakcheck: unknown option '$1'" >&2
        exit 2
        ;;
    *)
        SECTIONS="$SECTIONS $1"
        shift
        ;;
    esac
done

# lib.sh leaves us in c/, which is where run-tests.sh expects to run from
# (it reaches the corpus as ../tests/...). REPO is the repository root, which
# is where std/ and tests/ actually live.
CDIR=$(pwd)
REPO=$(cd .. && pwd)
ASAN_BUILD_DIR=build/asan
: "${SALAM:=$ASAN_BUILD_DIR/salam}"
: "${LSAN_SUPP:=tools/lsan.supp}"

# ---------------------------------------------------------------------------
# Build (opt-in). Deliberately WITH_LLVM=0: the LLVM path is covered by its
# own leakcheck run, and linking LLVM here would drag ~1 GB of development
# packages into what should be a two-minute build.
# ---------------------------------------------------------------------------
if [ "$DO_BUILD" -eq 1 ]; then
    echo "== building ASan salam ($ASAN_BUILD_DIR/salam) =="
    ASAN_CFLAGS="-O1 -g -fsanitize=address -fno-omit-frame-pointer"
    ASAN_CFLAGS="$ASAN_CFLAGS -std=gnu89 -Wall -Wextra -Wno-unused-parameter"
    ASAN_CFLAGS="$ASAN_CFLAGS -Wno-unused-function -Wno-unused-variable"
    make -j"${NPROC:-4}" \
        CC="${CC:-gcc}" \
        WITH_LLVM="${WITH_LLVM:-0}" WITH_LLD="${WITH_LLD:-0}" \
        BUILD_DIR="$ASAN_BUILD_DIR" OUTDIR="$CDIR/$ASAN_BUILD_DIR" \
        CFLAGS="$ASAN_CFLAGS" LDFLAGS="-fsanitize=address" ||
        {
            echo "leakcheck: ASan build failed" >&2
            exit 1
        }
fi

case "$SALAM" in
/* | [A-Za-z]:*) SALAM_ABS="$SALAM" ;;
*) SALAM_ABS="$CDIR/$SALAM" ;;
esac
[ -x "$SALAM_ABS" ] || {
    echo "leakcheck: no salam at '$SALAM_ABS' (run with --build, or set SALAM)" >&2
    exit 2
}

case "$LSAN_SUPP" in
/* | [A-Za-z]:*) SUPP_ABS="$LSAN_SUPP" ;;
*) SUPP_ABS="$CDIR/$LSAN_SUPP" ;;
esac
[ -f "$SUPP_ABS" ] || {
    echo "leakcheck: no suppression file at '$SUPP_ABS'" >&2
    exit 2
}

# A salam without -fsanitize=address exits cleanly no matter how much it
# leaks, so a misconfigured job would report a perfect score forever. Prove
# the instrumentation is really there before trusting a single result.
if ! (strings "$SALAM_ABS" 2>/dev/null || cat "$SALAM_ABS") |
    grep -q '__asan_init\|AddressSanitizer'; then
    echo "leakcheck: '$SALAM_ABS' is not an AddressSanitizer build." >&2
    echo "           A clean run from an uninstrumented binary proves nothing." >&2
    exit 2
fi

if [ -z "$REPORTS" ]; then
    REPORTS=${TMPDIR:-/tmp}/salam-leak.$$
fi
rm -rf "$REPORTS"
mkdir -p "$REPORTS"
case "$REPORTS" in
/* | [A-Za-z]:*) ;;
*) REPORTS="$CDIR/$REPORTS" ;;
esac

# exitcode=0: a leak must NOT fail the process it was found in. Otherwise the
# test that leaked is graded FAIL, run-tests.sh stops being a usable signal,
# and the leak itself is buried under a wall of unrelated diffs. The reports
# on disk are the verdict; the tests keep grading correctness only.
ASAN_OPTIONS="detect_leaks=1:exitcode=0:log_path=$REPORTS/leak"
ASAN_OPTIONS="$ASAN_OPTIONS:suppressions=$SUPP_ABS:print_suppressions=0"
ASAN_OPTIONS="$ASAN_OPTIONS:max_leaks=200:fast_unwind_on_malloc=0"
export ASAN_OPTIONS
export SALAM="$SALAM_ABS"
# run-tests.sh only auto-sets SALAM_STD when ./std exists, and from c/ it does
# not - so an ASan salam living under c/build/asan, rather than beside the
# repository's std/, would resolve no stdlib at all and every test would fail
# for a reason that has nothing to do with leaks.
if [ -z "${SALAM_STD:-}" ] && [ -d "$REPO/std" ]; then
    SALAM_STD="$REPO/std"
    export SALAM_STD
fi

echo "salam       : $SALAM_ABS"
echo "suppressions: $SUPP_ABS"
echo "reports     : $REPORTS"
echo

suite_rc=0
if [ "$DO_SUITE" -eq 1 ]; then
    echo "== run-tests.sh under LeakSanitizer =="
    # shellcheck disable=SC2086 # both are deliberately word-split flag lists
    sh tools/bash/run-tests.sh $NPROC_ARG $SECTIONS
    suite_rc=$?
    echo
fi

if [ "$DO_SWEEP" -eq 1 ]; then
    echo "== extra-surface sweep under LeakSanitizer =="
    sh tools/bash/leakcheck-sweep.sh "$SALAM_ABS"
    echo
fi

# ---------------------------------------------------------------------------
# Verdict. One file per leaking process, so the count is "how many salam
# invocations leaked", and the files themselves say where.
# ---------------------------------------------------------------------------
nleaks=$(find "$REPORTS" -type f 2>/dev/null | wc -l | tr -d ' ')
echo "========================================"
if [ "$nleaks" -eq 0 ]; then
    echo "LEAKCHECK: clean - 0 leaking salam invocations"
else
    echo "LEAKCHECK: $nleaks leaking salam invocation(s)"
    echo
    # Distinct allocation sites, not distinct processes: the same leak in
    # 300 tests is one thing to fix, and printing it 300 times helps nobody.
    echo "--- distinct allocation sites ---"
    grep -h '^    #[0-9]* 0x' "$REPORTS"/* 2>/dev/null |
        sed 's/^ *#[0-9]* 0x[0-9a-f]* in //; s/ (.*//; s/+0x[0-9a-f]*$//' |
        grep -v '^__libc_start\|^_start\|asan_malloc_linux\|^malloc$\|^calloc$\|^realloc$' |
        sort | uniq -c | sort -rn | head -40
    echo
    echo "--- first full report ($(find "$REPORTS" -type f | sed 1q)) ---"
    head -60 "$(find "$REPORTS" -type f | sed 1q)"
    echo
    echo "(all reports are under $REPORTS)"
fi
echo "========================================"

rc=0
if [ "$nleaks" -gt "$ALLOW" ]; then
    echo "leakcheck: FAILED - $nleaks leaking invocation(s), budget is $ALLOW" >&2
    rc=1
elif [ "$nleaks" -lt "$ALLOW" ]; then
    echo "leakcheck: budget can be lowered: $nleaks < $ALLOW" >&2
fi
if [ "$suite_rc" -ne 0 ]; then
    echo "leakcheck: note - run-tests.sh itself exited $suite_rc" >&2
fi
exit "$rc"
