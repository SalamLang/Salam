#!/bin/sh
# Builds tests with --backend=native for another target and runs them under an
# emulator, comparing the output with the .out files.
#
# The executable is linked by the built-in linker against a musl sysroot (set
# SALAM_MUSL_SYSROOT, or let tools/bash/fetch-musl-sysroots.sh fill the
# cache). Tests run from the repository root, since several read fixtures by
# relative path. Output is compared the way run-tests.sh compares it, ignoring
# trailing newlines.
#
# Some tests cannot pass on a cross target and are skipped: they need -D flags
# from their test harness, a database library, or a trapping integer division
# (release builds on ARM return 0), and on 32-bit targets a few assume a
# 64-bit usize or declare C's int-returning abs() as i64. stdlib/os_platform_demo
# runs a different number of assertions per architecture, so its .out only
# matches on x86-64. NATIVE_CROSS_SKIP replaces the list.
#
# Each program gets NATIVE_CROSS_TIMEOUT seconds (180 by default); the larger
# tensor and nn demos take over a minute under qemu.
#
# Usage: tools/bash/native-cross-check.sh <salam> <triple> <runner> [group...]
#   e.g. tools/bash/native-cross-check.sh ./salam aarch64-linux-musl qemu-aarch64 basics general
#   Use `env` as the runner for a target the host runs directly (i686).

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
TRIPLE=$2
RUNNER=$3
shift 3
TEST_GROUPS=${*:-basics features types data games match switch llvm general stdlib}
DEFAULT_SKIP="general/condcomp_toplevel general/condcomp_pruned_lints general/const_define general/preproc
general/dbfile general/dbparams general/dbtest general/linkkind general/sqlite_*
llvm/ll_div_zero_runtime llvm/ll_mod_zero_compound stdlib/os_platform_demo"
case "$TRIPLE" in
aarch64* | arm64* | riscv*) ;;
*) DEFAULT_SKIP="$DEFAULT_SKIP general/div_zero_compound general/div_zero_runtime general/mod_zero_runtime_i64" ;;
esac
case "$TRIPLE" in
i686* | i386* | arm-* | armv7*) DEFAULT_SKIP="$DEFAULT_SKIP types/size_type llvm/size_type general/extern_builtin_protos" ;;
esac
SKIP=${NATIVE_CROSS_SKIP-$DEFAULT_SKIP}
LIMIT=${NATIVE_CROSS_TIMEOUT:-180}
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
ok=0
bad=0
skipped=0
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
for grp in $TEST_GROUPS; do
    for f in "$ROOT"/tests/en/"$grp"/*.salam; do
        exp=${f%.salam}.out
        [ -f "$exp" ] || continue
        id=$grp/$(basename "$f" .salam)
        if skip_of "$id"; then
            skipped=$((skipped + 1))
            continue
        fi
        exe=$work/t
        rm -f "$exe"
        if ! (cd "$work" && "$SALAM" build "$f" --backend=native --release --target="$TRIPLE" --output="$exe" --log-level=error >"$work/build.log" 2>&1); then
            echo "BUILDFAIL $id"
            bad=$((bad + 1))
            continue
        fi
        (cd "$ROOT" && timeout "$LIMIT" $RUNNER "$exe" </dev/null >"$work/got" 2>&1)
        if [ "$(tr -d '\r' <"$work/got")" = "$(tr -d '\r' <"$exp")" ]; then
            ok=$((ok + 1))
        else
            echo "DIFF $id"
            bad=$((bad + 1))
        fi
    done
done
echo "native cross check ($TRIPLE): $ok ok, $bad differ, $skipped skipped"
[ "$bad" -eq 0 ]
