#!/bin/sh
# Builds tests with --backend=native for another target and runs them under an
# emulator, comparing the output with the .out files.
#
# The executable is linked by the built-in linker against a musl sysroot (set
# SALAM_MUSL_SYSROOT, or let tools/bash/fetch-musl-sysroots.sh fill the
# cache). Tests run from the repository root, since several read fixtures by
# relative path. Tests that need -D flags, a database library or a trapping
# integer division (release builds on ARM return 0) are expected to differ.
#
# Usage: tools/bash/native-cross-check.sh <salam> <triple> <runner> [group...]
#   e.g. tools/bash/native-cross-check.sh ./salam aarch64-linux-musl qemu-aarch64 basics general

set -u
SALAM=$1
TRIPLE=$2
RUNNER=$3
shift 3
TEST_GROUPS=${*:-basics features types data games match switch llvm general stdlib}
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
ok=0
bad=0
for grp in $TEST_GROUPS; do
    for f in "$ROOT"/tests/en/"$grp"/*.salam; do
        exp=${f%.salam}.out
        [ -f "$exp" ] || continue
        id=$grp/$(basename "$f" .salam)
        exe=$work/t
        rm -f "$exe"
        if ! (cd "$work" && "$SALAM" build "$f" --backend=native --release --target="$TRIPLE" --output="$exe" --log-level=error >"$work/build.log" 2>&1); then
            echo "BUILDFAIL $id"
            bad=$((bad + 1))
            continue
        fi
        (cd "$ROOT" && timeout 60 $RUNNER "$exe" </dev/null >"$work/got" 2>&1)
        if cmp -s "$work/got" "$exp"; then
            ok=$((ok + 1))
        else
            echo "DIFF $id"
            bad=$((bad + 1))
        fi
    done
done
echo "native cross check ($TRIPLE): $ok ok, $bad differ"
[ "$bad" -eq 0 ]
