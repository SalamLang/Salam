#!/bin/sh

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

CDIR=$(pwd)
REPO=$CDIR
ASAN_BUILD_DIR=build/asan
: "${SALAM:=$ASAN_BUILD_DIR/salam}"
: "${LSAN_SUPP:=tools/lsan.supp}"

if [ "$DO_BUILD" -eq 1 ]; then
    echo "== building ASan salam ($ASAN_BUILD_DIR/salam) =="
    SEED=${SALAM_SEED:-salam}
    command -v "$SEED" >/dev/null 2>&1 || [ -x "$SEED" ] || {
        echo "leakcheck: no seed compiler '$SEED' (install one with install.sh, or set SALAM_SEED)" >&2
        exit 1
    }
    mkdir -p "$ASAN_BUILD_DIR"
    "$SEED" build compiler/main.salam --output="$CDIR/$ASAN_BUILD_DIR/salam" \
        --asan --debug-info --cc="${CC:-cc}" ||
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

LSAN_SUPPORTED=1
if ASAN_OPTIONS=detect_leaks=1 "$SALAM_ABS" version 2>&1 |
    grep -q 'detect_leaks is not supported'; then
    LSAN_SUPPORTED=0
    echo "leakcheck: warning - LeakSanitizer is not supported on this platform;" >&2
    echo "  every run will abort and no leak can be detected. Use Linux." >&2
fi

ASAN_OPTIONS="detect_leaks=1:exitcode=0:log_path=$REPORTS/leak"
ASAN_OPTIONS="$ASAN_OPTIONS:max_leaks=200:fast_unwind_on_malloc=0"
export ASAN_OPTIONS
LSAN_OPTIONS="suppressions=$SUPP_ABS:print_suppressions=0"
export LSAN_OPTIONS
export SALAM="$SALAM_ABS"
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

nleaks=$(find "$REPORTS" -type f 2>/dev/null | wc -l | tr -d ' ')
echo "========================================"
if [ "$LSAN_SUPPORTED" -eq 0 ]; then
    echo "LEAKCHECK: INCONCLUSIVE - LeakSanitizer is unsupported here, nothing was checked"
    echo "  (ASan aborts every run with 'detect_leaks is not supported on this"
    echo "   platform'; run this on Linux for a real answer)"
    exit 2
elif [ "$nleaks" -eq 0 ]; then
    echo "LEAKCHECK: clean - 0 sanitizer reports"
else
    echo "LEAKCHECK: $nleaks salam invocation(s) reported a leak or memory error"
    echo
    echo "--- distinct allocation sites ---"
    find "$REPORTS" -type f -print0 2>/dev/null |
        xargs -0 grep -h '^    #[0-9]* 0x' 2>/dev/null |
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
