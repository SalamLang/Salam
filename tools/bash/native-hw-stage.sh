#!/bin/sh
# Builds what the gate's real-hardware jobs run besides their test groups, for
# tools/bash/native-stage-tests.sh to run on a machine of the target's kind:
#   <dir>/abi       the C ABI and timer tests, with the native backend
#   <dir>/abi-llvm  the C ABI tests again, with the LLVM backend
#   <dir>/icf       a few programs linked with --icf, which all fold functions
#                   on some target
# The C ABI tests pass and return structs across C code, in fixed and variadic
# arguments, so a calling convention mistake only shows where the C side is the
# real one. Their C helpers need clang and llvm-ar on PATH.
#
# Usage: tools/bash/native-hw-stage.sh <salam> <triple> <dir>

set -eu
SALAM=$1
TRIPLE=$2
DIR=$3
HERE=$(dirname "$0")
ABI="general/extern_variadic_va_struct general/extern_struct_abi general/extern_struct_byval
general/extern_struct_overflow general/extern_variadic_struct interop/ffi_struct_value"
ICF="stdlib/archive_demo stdlib/json_errors_demo stdlib/autograd_demo
llvm/ll_hashmap llvm/generic_struct_pkg_types llvm/ll_closure_counter"
if ! command -v clang >/dev/null || ! command -v llvm-ar >/dev/null; then
    echo "native-hw-stage: clang and llvm-ar are needed for the C helpers" >&2
    exit 1
fi
# shellcheck disable=SC2086
NATIVE_CROSS_STAGE=$DIR/abi sh "$HERE/native-cross-check.sh" "$SALAM" "$TRIPLE" none $ABI stdlib/time_timer_demo
# shellcheck disable=SC2086
NATIVE_CROSS_STAGE=$DIR/abi-llvm NATIVE_CROSS_FLAGS=--backend=llvm sh "$HERE/native-cross-check.sh" "$SALAM" "$TRIPLE" none $ABI
# shellcheck disable=SC2086
NATIVE_CROSS_STAGE=$DIR/icf NATIVE_CROSS_FLAGS=--icf sh "$HERE/native-cross-check.sh" "$SALAM" "$TRIPLE" none $ICF
