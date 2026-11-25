#!/bin/sh

set -eu

here=$(cd "$(dirname "$0")" && pwd)
salam=${SALAM:-salam}
cc=${CC:-clang}
out=${OUT:-$here/kernel.elf}

cd "$here"

"$cc" --target=riscv64-unknown-none-elf -march=rv64gc -mabi=lp64d -c boot.s -o boot.o

"$salam" build ../main.salam \
    --target=riscv64-unknown-none \
    --entry=_start \
    --linker-script=linker.ld \
    --link-object=boot.o \
    --output="$out" \
    --log-level=error

echo "kernel: $out"
