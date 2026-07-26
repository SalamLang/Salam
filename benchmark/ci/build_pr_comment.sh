#!/usr/bin/env bash
# Builds the sticky benchmark PR comment body from benchmark/RESULTS.md.
#
# Usage: build_pr_comment.sh <RESULTS.md path>
# Env:   SHORT_SHA, RUN_URL, ARTIFACT_NAME (all optional, used for footer links)
#

set -euo pipefail

RESULTS_FILE="${1:?usage: build_pr_comment.sh <RESULTS.md>}"
SHORT_SHA="${SHORT_SHA:-unknown}"
RUN_URL="${RUN_URL:-}"
ARTIFACT_NAME="${ARTIFACT_NAME:-}"

MARKER='<!-- salam-benchmark-report -->'
MAX_BODY_CHARS=60000

quick_section="$(awk '
    /^## Per-program results/ { exit }
    /^## Overall ranking/ { flag=1 }
    flag { print }
' "$RESULTS_FILE")"

build_full() {
    echo "$MARKER"
    echo "## Benchmark results — commit \`${SHORT_SHA}\`"
    echo
    if [ -n "$quick_section" ]; then
        echo "$quick_section"
    else
        echo "_(overall ranking section not found in RESULTS.md)_"
    fi
    echo
    echo "<details>"
    echo "<summary>Full report — all programs, environment, methodology (click to expand)</summary>"
    echo
    cat "$RESULTS_FILE"
    echo
    echo "</details>"
    echo
    if [ -n "$RUN_URL" ]; then
        echo "[Workflow run & job summary](${RUN_URL})"
    fi
}

build_truncated() {
    echo "$MARKER"
    echo "## Benchmark results — commit \`${SHORT_SHA}\`"
    echo
    if [ -n "$quick_section" ]; then
        echo "$quick_section"
    else
        echo "_(overall ranking section not found in RESULTS.md)_"
    fi
    echo
    echo "_The full per-program report is too large to inline in a PR comment._"
    if [ -n "$RUN_URL" ]; then
        echo "_See the full report in the [job summary](${RUN_URL})${ARTIFACT_NAME:+ or download the \`${ARTIFACT_NAME}\` artifact from that run}._"
    fi
}

full_body="$(build_full)"
body_len=$(printf '%s' "$full_body" | wc -c)

if [ "$body_len" -gt "$MAX_BODY_CHARS" ]; then
    build_truncated
else
    printf '%s\n' "$full_body"
fi
