#!/bin/sh
# Runs test programs that tools/bash/native-cross-check.sh staged with
# NATIVE_CROSS_STAGE=<dir> on the machine they were built for (macOS, Windows,
# AArch64 Linux). Each <dir>/<group>/<name>[.exe] runs from the repository
# root, like the cross check runs them, and its output is compared with
# tests/en/<group>/<name>.out, ignoring carriage returns and trailing newlines.
#
# NATIVE_STAGE_SKIP lists group/name patterns that do not apply on this kind
# of machine; NATIVE_CROSS_TIMEOUT is the time each program gets (180 by
# default). NATIVE_STAGE_RUNNER runs each program through a command such as
# wine, to try a stage out away from its machine.
#
# Usage: tools/bash/native-stage-tests.sh <dir>

set -u
DIR=$(cd "$1" && pwd)
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
LIMIT=${NATIVE_CROSS_TIMEOUT:-180}
SKIP=${NATIVE_STAGE_SKIP:-}
RUNNER=${NATIVE_STAGE_RUNNER:-}
ok=0
bad=0
skipped=0
got=$(mktemp)
trap 'rm -f "$got"' EXIT
skip_of() {
    set -f
    for pat in $SKIP; do
        # shellcheck disable=SC2254
        case "$1" in $pat)
            set +f
            return 0
            ;;
        esac
    done
    set +f
    return 1
}
limited() {
    if command -v timeout >/dev/null 2>&1; then
        timeout "$LIMIT" "$@"
    else
        perl -e 'alarm shift; exec @ARGV' "$LIMIT" "$@"
    fi
}
for exe in "$DIR"/*/*; do
    [ -f "$exe" ] || continue
    grp=$(basename "$(dirname "$exe")")
    name=$(basename "$exe" .exe)
    id=$grp/$name
    exp=$ROOT/tests/en/$id.out
    [ -f "$exp" ] || continue
    if skip_of "$id"; then
        skipped=$((skipped + 1))
        continue
    fi
    chmod +x "$exe" 2>/dev/null
    # shellcheck disable=SC2086
    (cd "$ROOT" && limited $RUNNER "$exe" </dev/null >"$got" 2>&1)
    if [ "$(tr -d '\r' <"$got")" = "$(tr -d '\r' <"$exp")" ]; then
        ok=$((ok + 1))
    else
        echo "DIFF $id"
        diff "$exp" "$got" | head -6
        bad=$((bad + 1))
    fi
done
echo "native staged tests: $ok ok, $bad differ, $skipped skipped"
[ "$bad" -eq 0 ] && [ "$ok" -gt 0 ]
