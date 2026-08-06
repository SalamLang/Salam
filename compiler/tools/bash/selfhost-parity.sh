#!/bin/sh
# Self-host parity gate.
#
# Runs the shared test corpus (tests/{en,fa,ar}) twice over the same
# machine in the same job - once with a reference compiler, once with the
# self-hosted compiler built from compiler/*.salam - and fails when the
# self-hosted one falls further behind than compiler/tools/selfhost-parity-budget.txt
# allows.
#
# Why a delta and not an absolute pass count: a lot of the corpus depends
# on host facilities (sqlite3/mariadb headers, loopback networking, what
# the chosen --cc= can codegen). Those fail for BOTH compilers, and the
# count moves with the runner image. Diffing two runs in one job cancels
# all of that out, so the only thing this gate can ever go red on is the
# self-hosted compiler genuinely regressing against the reference.
#
# Usage:
#   compiler/tools/bash/selfhost-parity.sh <ref-salam> <selfhost-salam> [suite ...]
#
# With no suites, runs the default set (every suite named in the budget file).
# Run from the repository root.

set -eu

ROOT=$(cd "$(dirname "$0")/../../.." && pwd)
BUDGET_FILE="$ROOT/compiler/tools/selfhost-parity-budget.txt"

if [ $# -lt 2 ]; then
    echo "usage: $0 <ref-salam> <selfhost-salam> [suite ...]" >&2
    exit 2
fi

REF=$1
NEW=$2
shift 2

for bin in "$REF" "$NEW"; do
    # run-tests.sh's lib.sh silently falls back to building the C compiler
    # when $SALAM is not executable, which would make this script compare a
    # compiler against itself and report a perfect score. Refuse instead.
    [ -x "$bin" ] || {
        echo "error: not an executable: $bin" >&2
        exit 2
    }
done
REF=$(cd "$(dirname "$REF")" && pwd)/$(basename "$REF")
NEW=$(cd "$(dirname "$NEW")" && pwd)/$(basename "$NEW")

[ -f "$BUDGET_FILE" ] || {
    echo "error: missing $BUDGET_FILE" >&2
    exit 2
}

budget_for() {
    awk -v s="$1" '$1 == s { print $2; found = 1 } END { if (!found) print "" }' \
        "$BUDGET_FILE" | sed 's/[^0-9]//g'
}

if [ $# -gt 0 ]; then
    SUITES="$*"
else
    SUITES=$(grep -vE '^[[:space:]]*(#|$)' "$BUDGET_FILE" | awk '{ print $1 }')
fi

# run-tests.sh resolves the corpus as ../tests and expects to be run from c/.
run_suite() {
    # $1 = salam binary, $2 = suite. Echoes "<passed> <failed>".
    log="$WORK/$(basename "$1").$2.log"
    (
        cd "$ROOT/c" || exit 1
        SALAM="$1" SALAM_STD="$ROOT/std" \
            sh tools/bash/run-tests.sh "$2" >"$log" 2>&1 || true
    )
    # A missing/garbled RESULT line means the run died before summarising -
    # treat it as a hard error rather than silently scoring it 0 0 (which
    # would read as a clean sweep and pass the gate).
    line=$(grep -E '^RESULT: [0-9]+ passed, [0-9]+ failed' "$log" | tail -1 || true)
    if [ -z "$line" ]; then
        echo "::error::no RESULT line from $2 with $1 - last 30 lines follow" >&2
        tail -30 "$log" >&2
        return 1
    fi
    echo "$line" | sed -E 's/^RESULT: ([0-9]+) passed, ([0-9]+) failed.*/\1 \2/'
}

WORK=$(mktemp -d 2>/dev/null || mktemp -d -t salam-parity)
trap 'rm -rf "$WORK"' EXIT INT TERM

echo "reference : $REF"
"$REF" version 2>&1 | head -1 || true
echo "self-host : $NEW"
"$NEW" version 2>&1 | head -1 || true
echo "suites    : $SUITES"
echo

status=0
slack_found=0
printf '%-12s %14s %14s %8s %8s  %s\n' SUITE REFERENCE SELF-HOST DELTA BUDGET RESULT
printf -- '---------------------------------------------------------------------------\n'

for suite in $SUITES; do
    budget=$(budget_for "$suite")
    if [ -z "$budget" ]; then
        echo "::error::suite '$suite' has no entry in $BUDGET_FILE" >&2
        status=1
        continue
    fi

    # "<passed> <failed>" split without word-splitting a command
    # substitution, which shellcheck flags and which would silently score a
    # crashed run as 0 0 anyway.
    ref_counts=$(run_suite "$REF" "$suite") || exit 1
    ref_pass=${ref_counts% *}
    ref_fail=${ref_counts#* }
    new_counts=$(run_suite "$NEW" "$suite") || exit 1
    new_pass=${new_counts% *}
    new_fail=${new_counts#* }

    delta=$((new_fail - ref_fail))
    if [ "$delta" -gt "$budget" ]; then
        verdict="FAIL (over budget by $((delta - budget)))"
        status=1
    elif [ "$delta" -lt "$budget" ]; then
        verdict="ok - budget can drop to $delta"
        slack_found=1
    else
        verdict="ok"
    fi

    printf '%-12s %6s p / %3s f %6s p / %3s f %8s %8s  %s\n' \
        "$suite" "$ref_pass" "$ref_fail" "$new_pass" "$new_fail" \
        "$delta" "$budget" "$verdict"
done

echo
if [ "$status" -ne 0 ]; then
    echo "::error::self-hosted compiler regressed against the reference compiler."
    echo "Investigate the failing suites above, or - if the new failures are"
    echo "understood and accepted - raise the matching line in"
    echo "compiler/tools/selfhost-parity-budget.txt (and say why in the PR)."
elif [ "$slack_found" -ne 0 ]; then
    echo "::warning::parity improved - lower the slack lines in compiler/tools/selfhost-parity-budget.txt to lock the gain in."
else
    echo "Self-host parity holds at budget."
fi

exit "$status"
