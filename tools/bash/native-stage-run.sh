#!/bin/sh
# Runs the programs tools/bash/native-windows-check.sh staged with
# NATIVE_WIN_STAGE=<dir>: each <name>.exe in <dir> is run from <dir> and its
# stdout compared with <name>.out, ignoring carriage returns. Meant for the
# machine the programs were built for, such as a Windows on Arm runner.
#
# Usage: tools/bash/native-stage-run.sh <dir> [runner]

set -u
DIR=$1
RUNNER=${2:-}
pass=0
fail=0
for exe in "$DIR"/*.exe; do
    [ -f "$exe" ] || continue
    b=$(basename "$exe" .exe)
    exp="$DIR/$b.out"
    [ -f "$exp" ] || continue
    # shellcheck disable=SC2086
    (cd "$DIR" && timeout 120 $RUNNER "./$b.exe" </dev/null >"$b.got" 2>"$b.err")
    rc=$?
    if [ "$(tr -d '\r' <"$DIR/$b.got")" = "$(tr -d '\r' <"$exp")" ]; then
        pass=$((pass + 1))
    else
        echo "FAIL $b (exit $rc)"
        head -3 "$DIR/$b.err"
        diff "$exp" "$DIR/$b.got" | head -6
        fail=$((fail + 1))
    fi
done
echo "staged run: $pass passed, $fail failed"
[ "$fail" -eq 0 ] && [ "$pass" -gt 0 ]
