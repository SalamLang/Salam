#!/bin/sh
# Compares the native backend with the LLVM backend on build time and
# executable size, for the compiler itself and a few large test programs, and
# prints the result as a markdown table (CI appends it to the job summary).
#
# Every program is built with --release for the host, once per backend and
# NATIVE_REPORT_RUNS times each (1 by default, the fastest run counts). The
# LLVM column is whatever --backend=llvm does with this compiler: a compiler
# without LLVM built in hands its IR to the clang on PATH, whose version is
# printed under the table. Sizes are the executables as written, unstripped.
#
# Single rows are noisy on shared machines, so the gate looks at the totals
# only, and as ratios of the two backends measured side by side: it fails
# when the native backend takes longer than NATIVE_REPORT_MAX_TIME_RATIO times
# LLVM's total time (0.6 by default) or writes more than
# NATIVE_REPORT_MAX_SIZE_RATIO times LLVM's total size (1.5 by default), or
# when either backend cannot build a program. Set a ratio to 0 to only report.
#
# Usage: tools/bash/native-compile-report.sh <salam> [file.salam...]

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
shift
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
RUNS=${NATIVE_REPORT_RUNS:-1}
MAX_TIME=${NATIVE_REPORT_MAX_TIME_RATIO:-0.6}
MAX_SIZE=${NATIVE_REPORT_MAX_SIZE_RATIO:-1.5}
if [ $# -eq 0 ]; then
    set -- "$ROOT/compiler/main.salam" "$ROOT/tests/en/stdlib/tls_selftest.salam" \
        "$ROOT/tests/en/stdlib/image_demo.salam" "$ROOT/tests/en/stdlib/graphql_demo.salam" \
        "$ROOT/tests/en/stdlib/aws_s3_loopback.salam" "$ROOT/tests/en/general/swagger_serve.salam"
fi
export SALAM_STD="${SALAM_STD:-$ROOT/std}"
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
now_ms() {
    t=$(date +%s%N 2>/dev/null)
    case "$t" in
    *N | "") perl -MTime::HiRes=time -e 'printf "%d\n", time * 1000' ;;
    *) echo $((t / 1000000)) ;;
    esac
}
build() {
    best=
    i=0
    while [ "$i" -lt "$RUNS" ]; do
        rm -f "$work/out"
        s=$(now_ms)
        if ! (cd "$work" && "$SALAM" build "$1" --backend="$2" --release --output="$work/out" --log-level=error >"$work/log" 2>&1); then
            echo "native compile report: $(basename "$1") does not build with --backend=$2:" >&2
            head -5 "$work/log" >&2
            return 1
        fi
        e=$(now_ms)
        t=$((e - s))
        if [ -z "$best" ] || [ "$t" -lt "$best" ]; then best=$t; fi
        i=$((i + 1))
    done
    echo "$best $(wc -c <"$work/out" | tr -d ' ')"
}
secs() { awk -v m="$1" 'BEGIN { printf "%.2f s", m / 1000 }'; }
kib() { awk -v b="$1" 'BEGIN { printf "%.0f KiB", b / 1024 }'; }
ratio() { awk -v a="$1" -v b="$2" 'BEGIN { if (b > 0) printf "%.2f", a / b; else print "-" }'; }

failed=0
nt=0
lt=0
ns=0
lsz=0
rows=
for f in "$@"; do
    case "$f" in "$ROOT"/*) name=${f#"$ROOT"/} ;; *) name=$f ;; esac
    if ! n=$(build "$f" native) || ! l=$(build "$f" llvm); then
        failed=1
        rows="$rows| $name | build failed | | | | | |
"
        continue
    fi
    n_ms=${n% *}
    n_sz=${n#* }
    l_ms=${l% *}
    l_sz=${l#* }
    nt=$((nt + n_ms))
    lt=$((lt + l_ms))
    ns=$((ns + n_sz))
    lsz=$((lsz + l_sz))
    rows="$rows| $name | $(secs "$n_ms") | $(secs "$l_ms") | $(ratio "$n_ms" "$l_ms") | $(kib "$n_sz") | $(kib "$l_sz") | $(ratio "$n_sz" "$l_sz") |
"
done
time_ratio=$(ratio "$nt" "$lt")
size_ratio=$(ratio "$ns" "$lsz")
echo "### Native backend: build time and size against LLVM"
echo ""
echo "| Program | native build | LLVM build | time ratio | native size | LLVM size | size ratio |"
echo "| --- | ---: | ---: | ---: | ---: | ---: | ---: |"
printf '%s' "$rows"
echo "| **total** | **$(secs "$nt")** | **$(secs "$lt")** | **$time_ratio** | **$(kib "$ns")** | **$(kib "$lsz")** | **$size_ratio** |"
echo ""
echo "Release builds for the host, best of $RUNS run(s); ratios are native over LLVM. LLVM: $(clang --version 2>/dev/null | head -1 || echo 'no clang on PATH')."
over() { awk -v r="$1" -v m="$2" 'BEGIN { exit !(m > 0 && r != "-" && r > m) }'; }
if over "$time_ratio" "$MAX_TIME"; then
    echo ""
    echo "**The native backend took $time_ratio times LLVM's build time, over the limit of $MAX_TIME.**"
    failed=1
fi
if over "$size_ratio" "$MAX_SIZE"; then
    echo ""
    echo "**The native backend's executables are $size_ratio times LLVM's size, over the limit of $MAX_SIZE.**"
    failed=1
fi
[ "$failed" -eq 0 ]
