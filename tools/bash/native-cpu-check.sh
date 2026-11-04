#!/bin/sh
# Builds the vectorizer and image tests at --cpu=x86-64-v2 and x86-64-v3 and
# compares what they print with the same expected output as the baseline build,
# since the gate's sweep only runs them at the default level. It also disassembles
# lvec_blend and lvec_avx2 to show each level reached the code generator: the
# baseline has neither pblendvb nor ymm registers, x86-64-v2 has pblendvb and
# x86-64-v3 has ymm registers, from --cpu and from SALAM_NATIVE_CPU.
#
# A level the CPU lacks is skipped with a warning, as its programs would stop
# with an illegal instruction. Only for an x86-64 Linux host.
#
# Usage: tools/bash/native-cpu-check.sh <salam>

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
TRIPLE=x86_64-linux-gnu
EXPECT_TESTS="image_captcha image_rtl_card"
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
fail=0

tests=
for f in "$ROOT"/tests/en/general/lvec_*.salam "$ROOT"/tests/en/general/slp_*.salam; do
    tests="$tests general/$(basename "$f" .salam)"
done
for t in image_demo image_font_demo image_jpeg_save; do
    tests="$tests stdlib/$t"
done

build() {
    src=$1
    out=$2
    shift 2
    (cd "$work" && "$SALAM" build "$src" --backend=native --release --target="$TRIPLE" --output="$out" --log-level=error "$@")
}

count_insn() {
    objdump -d "$1" | grep -c "$2"
}

want_insn() {
    # want_insn <label> <exe> <pattern> <yes|no>
    n=$(count_insn "$2" "$3")
    case "$4:$n" in
    yes:0)
        echo "FAIL $1: no $3 in the disassembly"
        fail=1
        ;;
    no:0) ;;
    no:*)
        echo "FAIL $1: $n instructions use $3"
        fail=1
        ;;
    esac
}

check_expect() {
    # check_expect <level> <name>, each .expect line must appear in the output
    src=$ROOT/tests/en/apps/$2.salam
    d=$work/$1-$2
    mkdir -p "$d"
    if ! (cd "$d" && "$SALAM" build "$src" --backend=native --release --target="$TRIPLE" --output="$d/t" \
        --log-level=error --cpu="$1") >"$d/build.log" 2>&1; then
        echo "BUILDFAIL apps/$2 at $1"
        head -3 "$d/build.log"
        fail=1
        return
    fi
    (cd "$d" && timeout 60 ./t </dev/null >"$d/got" 2>&1)
    while IFS= read -r line || [ -n "$line" ]; do
        [ -n "$line" ] || continue
        if ! grep -qF -- "$line" "$d/got"; then
            echo "DIFF apps/$2 at $1: missing '$line'"
            head -10 "$d/got"
            fail=1
            break
        fi
    done <"$ROOT/tests/en/apps/$2.expect"
}

BLEND=$ROOT/tests/en/general/lvec_blend.salam
AVX2=$ROOT/tests/en/general/lvec_avx2.salam
if build "$BLEND" "$work/base" --cpu=x86-64 && build "$AVX2" "$work/base2" --cpu=x86-64; then
    want_insn "baseline lvec_blend" "$work/base" pblendvb no
    want_insn "baseline lvec_avx2" "$work/base2" ymm no
else
    echo "BUILDFAIL the baseline programs"
    fail=1
fi

for level in x86-64-v2 x86-64-v3; do
    case "$level" in
    x86-64-v2) flag=sse4_1 src=$BLEND pat=pblendvb ;;
    *) flag=avx2 src=$AVX2 pat=ymm ;;
    esac
    if ! grep -qw "$flag" /proc/cpuinfo; then
        echo "::warning::this runner has no $flag, $level was not tested"
        continue
    fi
    if build "$src" "$work/flag" "--cpu=$level"; then
        want_insn "--cpu=$level" "$work/flag" "$pat" yes
    else
        echo "BUILDFAIL --cpu=$level"
        fail=1
    fi
    if SALAM_NATIVE_CPU=$level build "$src" "$work/env"; then
        want_insn "SALAM_NATIVE_CPU=$level" "$work/env" "$pat" yes
    else
        echo "BUILDFAIL SALAM_NATIVE_CPU=$level"
        fail=1
    fi
    # shellcheck disable=SC2086
    NATIVE_CROSS_FLAGS=--cpu=$level sh "$HERE/native-cross-check.sh" "$SALAM" "$TRIPLE" env $tests || fail=1
    for t in $EXPECT_TESTS; do
        check_expect "$level" "$t"
    done
done
[ "$fail" = 0 ] && echo "native cpu check: passed"
exit "$fail"
