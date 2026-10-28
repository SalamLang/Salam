#!/bin/sh
# Runs the pairs tools/bash/native-bench-stage.sh built: for each benchmark
# the output with and without the vectorizers must be the same, and the best
# of five wall times of each is printed with their ratio and the geometric
# mean. Only a differing output fails; the times are for reading.
#
# Usage: tools/bash/native-bench-run.sh <dir> [runner]

set -u
DIR=$1
RUNNER=${2:-}
fail=0
best() {
    b=999999999999
    for _ in 1 2 3 4 5; do
        t0=$(date +%s%N)
        # shellcheck disable=SC2086
        (cd "$DIR" && $RUNNER "./$1" >/dev/null 2>&1)
        t1=$(date +%s%N)
        t=$(((t1 - t0) / 1000))
        [ "$t" -lt "$b" ] && b=$t
    done
    echo "$b"
}
printf '%-36s %10s %10s %6s\n' benchmark "vec (us)" "scalar" ratio
logsum=0
n=0
for on in "$DIR"/*.on; do
    [ -f "$on" ] || continue
    b=$(basename "$on" .on)
    # shellcheck disable=SC2086
    (cd "$DIR" && $RUNNER "./$b.on" >"$b.on.txt" 2>&1)
    # shellcheck disable=SC2086
    (cd "$DIR" && $RUNNER "./$b.off" >"$b.off.txt" 2>&1)
    if ! cmp -s "$DIR/$b.on.txt" "$DIR/$b.off.txt"; then
        echo "FAIL $b: the vectorized build prints something else"
        diff "$DIR/$b.off.txt" "$DIR/$b.on.txt" | head -6
        fail=$((fail + 1))
        continue
    fi
    ton=$(best "$b.on")
    toff=$(best "$b.off")
    r=$(awk -v a="$ton" -v c="$toff" 'BEGIN { printf "%.3f", a / (c > 0 ? c : 1) }')
    logsum=$(awk -v s="$logsum" -v r="$r" 'BEGIN { printf "%.6f", s + log(r) }')
    n=$((n + 1))
    printf '%-36s %10d %10d %6s\n' "$b" "$ton" "$toff" "$r"
done
if [ "$n" -gt 0 ]; then
    awk -v s="$logsum" -v n="$n" 'BEGIN { printf "geometric mean (vec / scalar): %.3f over %d benchmarks\n", exp(s / n), n }'
fi
echo "native bench run: $n compared, $fail differ"
[ "$fail" -eq 0 ] && [ "$n" -gt 0 ]
