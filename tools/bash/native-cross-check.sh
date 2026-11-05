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
# stdlib/jit_demo and stdlib/jit_calls_demo need dlopen, which a static musl program cannot do.
# stdlib/os_api is skipped under qemu, which prints a message when the forked child aborts.
# Some tests cannot pass on a cross target and are skipped: they need -D flags
# from their test harness, a database library, or a trapping integer division
# (release builds on ARM return 0), and on 32-bit targets a few assume a
# 64-bit usize or declare C's int-returning abs() as i64. stdlib/os_platform_demo
# runs a different number of assertions per architecture, so its .out only
# matches on x86-64. NATIVE_CROSS_SKIP replaces the list.
#
# A test with a C file next to it (name.c beside name.salam) links that file as
# the static library lib<name>.a, compiled for the target with clang and
# archived with llvm-ar (NATIVE_CROSS_CC and NATIVE_CROSS_AR override them); it
# is skipped when either tool is missing.
#
# Each program gets NATIVE_CROSS_TIMEOUT seconds (180 by default); the larger
# tensor and nn demos take over a minute under qemu. NATIVE_CROSS_FLAGS adds
# build flags, such as --icf.
#
# With NATIVE_CROSS_STAGE=<dir> the programs are only built: each lands in
# <dir>/<group>/<name> (with .exe for Windows targets), for
# tools/bash/native-stage-tests.sh to run on a machine of the target's kind
# (macOS, Windows, AArch64 Linux). The runner argument is then ignored.
#
# Usage: tools/bash/native-cross-check.sh <salam> <triple> <runner> [group|group/name...]
#   e.g. tools/bash/native-cross-check.sh ./salam aarch64-linux-musl qemu-aarch64 basics general interop/ffi_libm
#   Use `env` as the runner for a target the host runs directly (i686).

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
TRIPLE=$2
RUNNER=$3
shift 3
TEST_GROUPS=${*:-basics features types data games match switch llvm general stdlib}
DEFAULT_SKIP="general/condcomp_toplevel general/condcomp_pruned_lints general/const_define general/preproc
general/dbfile general/dbparams general/dbtest general/linkkind general/sqlite_* stdlib/db_extra
llvm/ll_div_zero_runtime llvm/ll_mod_zero_compound llvm/ll_str_oom general/oom_* stdlib/os_platform_demo"
case "$TRIPLE" in
aarch64* | arm64* | riscv*) ;;
*) DEFAULT_SKIP="$DEFAULT_SKIP general/div_zero_compound general/div_zero_runtime general/mod_zero_runtime_i64" ;;
esac
case "$TRIPLE" in
i686* | i386* | arm-* | armv7*) DEFAULT_SKIP="$DEFAULT_SKIP types/size_type llvm/size_type general/extern_builtin_protos general/extern_int128" ;;
esac
case "$TRIPLE" in
*windows*) DEFAULT_SKIP="$DEFAULT_SKIP general/extern_builtin_protos general/extern_int128" ;;
esac
case "$TRIPLE" in
*musl*) DEFAULT_SKIP="$DEFAULT_SKIP stdlib/jit_demo stdlib/jit_calls_demo" ;;
esac
case "$RUNNER" in
qemu*) DEFAULT_SKIP="$DEFAULT_SKIP stdlib/os_api" ;;
esac
SKIP=${NATIVE_CROSS_SKIP-$DEFAULT_SKIP}
STAGE=${NATIVE_CROSS_STAGE:-}
EXT=
case "$TRIPLE" in *windows*) EXT=.exe ;; esac
if [ -n "$STAGE" ]; then
    mkdir -p "$STAGE"
    STAGE=$(cd "$STAGE" && pwd)
fi
LIMIT=${NATIVE_CROSS_TIMEOUT:-180}
XCC=${NATIVE_CROSS_CC:-clang}
XAR=${NATIVE_CROSS_AR:-llvm-ar}
XMARCH=
case "$TRIPLE" in riscv64*) XMARCH=-march=rv64gc ;; esac
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
for spec in $TEST_GROUPS; do
    case "$spec" in
    */*) set -- "$ROOT/tests/en/$spec.salam" ;;
    *) set -- "$ROOT"/tests/en/"$spec"/*.salam ;;
    esac
    for f in "$@"; do
        exp=${f%.salam}.out
        [ -f "$exp" ] || continue
        grp=$(basename "$(dirname "$f")")
        id=$grp/$(basename "$f" .salam)
        if skip_of "$id"; then
            skipped=$((skipped + 1))
            continue
        fi
        libarg=
        if [ -f "${f%.salam}.c" ]; then
            if ! command -v "$XCC" >/dev/null 2>&1 || ! command -v "$XAR" >/dev/null 2>&1; then
                echo "SKIP $id (its C helper needs $XCC and $XAR)"
                skipped=$((skipped + 1))
                continue
            fi
            rm -rf "$work/clib"
            mkdir -p "$work/clib"
            # shellcheck disable=SC2086
            if ! "$XCC" --target="$TRIPLE" $XMARCH -O2 -ffreestanding -fno-stack-protector -c "${f%.salam}.c" \
                -o "$work/clib/helper.o" >"$work/build.log" 2>&1 ||
                ! "$XAR" rcs "$work/clib/lib$(basename "$f" .salam).a" "$work/clib/helper.o" >>"$work/build.log" 2>&1; then
                echo "BUILDFAIL $id (C helper)"
                head -3 "$work/build.log"
                bad=$((bad + 1))
                continue
            fi
            libarg=--libpath=$work/clib
        fi
        exe=$work/t$EXT
        rm -f "$exe"
        # shellcheck disable=SC2086
        if ! (cd "$work" && "$SALAM" build "$f" --backend=native --release --target="$TRIPLE" --output="$exe" \
            --log-level=error ${libarg:+"$libarg"} ${NATIVE_CROSS_FLAGS:-} >"$work/build.log" 2>&1); then
            echo "BUILDFAIL $id"
            head -3 "$work/build.log"
            bad=$((bad + 1))
            continue
        fi
        if [ -n "$STAGE" ]; then
            mkdir -p "$STAGE/$grp"
            if cp "$exe" "$STAGE/$id$EXT"; then
                ok=$((ok + 1))
            else
                echo "STAGEFAIL $id"
                bad=$((bad + 1))
            fi
            continue
        fi
        (cd "$ROOT" && timeout "$LIMIT" $RUNNER "$exe" </dev/null >"$work/got" 2>&1)
        rc=$?
        if [ "$(tr -d '\r' <"$work/got")" = "$(tr -d '\r' <"$exp")" ]; then
            ok=$((ok + 1))
        else
            echo "DIFF $id (exit $rc)"
            tr -d '\r' <"$work/got" | diff "$exp" - | head -20
            bad=$((bad + 1))
        fi
    done
done
if [ -n "$STAGE" ]; then
    echo "native cross stage ($TRIPLE): $ok staged in $STAGE, $bad failed to build, $skipped skipped"
    [ "$bad" -eq 0 ]
    exit
fi
echo "native cross check ($TRIPLE): $ok ok, $bad differ, $skipped skipped"
[ "$bad" -eq 0 ]
