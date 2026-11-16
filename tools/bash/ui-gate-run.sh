#!/bin/sh
# Runs one section of tools/bash/run-tests.sh and fails only on tests that are
# not listed in tools/ui-gate-known-failures.txt, so the gate catches new
# regressions while the listed failures are fixed one by one.
#
# Usage:
#   tools/bash/ui-gate-run.sh SECTION LANG [SHARD]       e.g. page en 1/2
#   tools/bash/ui-gate-run.sh --list SECTION LANG        print the failing names
#
# Env:
#   SALAM           compiler to test (required)
#   GATE_TIMEOUT    seconds before the run is killed (default 1200)
#   PER_TEST_TIMEOUT  seconds one compiler call may take (default 150), so a
#                   compiler hang fails one test instead of the whole run

set -u

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
KNOWN=$ROOT/tools/ui-gate-known-failures.txt
LISTONLY=0
if [ "${1:-}" = "--list" ]; then
    LISTONLY=1
    shift
fi
SECTION=${1:?section}
LANG_=${2:?language}
SHARD=${3:-}
: "${SALAM:?set SALAM to the compiler}"
: "${GATE_TIMEOUT:=1200}"

LOG=$(mktemp)
WRAP=$(mktemp -d)
trap 'rm -rf "$LOG" "$LOG.failed" "$LOG.new" "$WRAP"' EXIT
printf '#!/bin/sh\nexec timeout %s %s "$@"\n' "${PER_TEST_TIMEOUT:-150}" "$SALAM" >"$WRAP/salam"
chmod +x "$WRAP/salam"
SALAM=$WRAP/salam
export SALAM
cd "$ROOT" || exit 1
SALAM_STD=${SALAM_STD:-$ROOT/std}
export SALAM_STD

if [ -n "$SHARD" ]; then
    LANGS=$LANG_ timeout "$GATE_TIMEOUT" sh tools/bash/run-tests.sh "--shard=$SHARD" "$SECTION" >"$LOG" 2>&1
else
    LANGS=$LANG_ timeout "$GATE_TIMEOUT" sh tools/bash/run-tests.sh "$SECTION" >"$LOG" 2>&1
fi
rc=$?
tail -40 "$LOG" >&2

if ! grep -qE '^RESULT: ' "$LOG"; then
    echo "::error::the $SECTION $LANG run did not finish (exit $rc)"
    exit 1
fi

sed -nE 's/^FAIL \[[ 0-9]+\/[0-9]+\] ([^ ]+).*/\1/p; s/^FAIL ([^ \[]+).*/\1/p' "$LOG" | sort -u >"$LOG.failed"

if [ "$LISTONLY" = 1 ]; then
    cat "$LOG.failed"
    exit 0
fi

grep -vxFf "$KNOWN" "$LOG.failed" >"$LOG.new" || true
total=$(wc -l <"$LOG.failed" | tr -d ' ')
fresh=$(wc -l <"$LOG.new" | tr -d ' ')
echo "failing: $total, not in the known list: $fresh"
if [ "$fresh" -gt 0 ]; then
    echo "::error::new failures in $SECTION $LANG"
    cat "$LOG.new"
    exit 1
fi
exit 0
