#!/bin/sh
# Builds test programs with an installed compiler on the machine it runs on
# and compares each program's stdout with its .out file, ignoring carriage
# returns. Nothing is cross-compiled, so this checks a compiler package as a
# user would run it, for example Salam for Windows on Arm on a Windows on Arm
# machine.
#
# NATIVE_HOST_FLAGS adds build flags, such as --backend=native.
#
# Usage: tools/bash/native-host-check.sh <salam> <file.salam>...

set -u
SALAM=$1
shift
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
FLAGS=${NATIVE_HOST_FLAGS:-}
limited() {
    if command -v timeout >/dev/null 2>&1; then
        timeout 120 "$@"
    else
        perl -e 'alarm 120; exec @ARGV' "$@"
    fi
}
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
pass=0
fail=0
for f in "$@"; do
    exp="${f%.salam}.out"
    [ -f "$exp" ] || continue
    b=$(basename "$f" .salam)
    # shellcheck disable=SC2086
    if ! "$SALAM" build "$f" $FLAGS --output="$work/$b.exe" --log-level=error >"$work/$b.log" 2>&1; then
        echo "FAIL $b (build)"
        head -5 "$work/$b.log"
        fail=$((fail + 1))
        continue
    fi
    (cd "$work" && limited "./$b.exe" </dev/null >"$b.got" 2>"$b.err")
    rc=$?
    if [ "$rc" -ne 124 ] && [ "$rc" -ne 142 ] && [ "$(tr -d '\r' <"$work/$b.got")" = "$(tr -d '\r' <"$exp")" ]; then
        pass=$((pass + 1))
    else
        echo "FAIL $b (exit $rc)"
        diff "$exp" "$work/$b.got" | head -6
        fail=$((fail + 1))
    fi
done
echo "host check: $pass passed, $fail failed"
[ "$fail" -eq 0 ] && [ "$pass" -gt 0 ]
