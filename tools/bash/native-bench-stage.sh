#!/bin/sh
# Builds every benchmark in benchmark/programs with --backend=native for a
# target twice, once with the vectorizers and once without (SALAM_NG_OFF),
# into <dir> as <name>.on and <name>.off, for tools/bash/native-bench-run.sh
# to run on a machine of that target.
#
# Usage: tools/bash/native-bench-stage.sh <salam> <triple> <dir>

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
TRIPLE=$2
mkdir -p "$3"
DIR=$(cd "$3" && pwd)
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
fail=0
n=0
for p in "$ROOT"/benchmark/programs/[0-9][0-9]_*/prog.salam; do
    b=$(basename "$(dirname "$p")")
    for mode in on off; do
        off=
        [ "$mode" = off ] && off=lvec,slp,rvec
        if ! (cd "$DIR" && SALAM_NG_OFF=$off "$SALAM" build "$p" --backend=native --release --target="$TRIPLE" \
            --output="$b.$mode" --log-level=error >"$b.$mode.log" 2>&1); then
            echo "FAIL $b ($mode): build failed"
            sed -n 1,5p "$DIR/$b.$mode.log"
            fail=$((fail + 1))
        fi
    done
    n=$((n + 1))
done
echo "native bench stage ($TRIPLE): $n benchmarks, $fail builds failed"
[ "$fail" -eq 0 ]
