#!/bin/sh

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

run_suite() {
    log="$WORK/$(basename "$1").$2.log"
    (
        cd "$ROOT" || exit 1
        SALAM="$1" SALAM_STD="$ROOT/std" \
            sh "$ROOT/tools/bash/run-tests.sh" "$2" >"$log" 2>&1 || true
    )
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
