#!/bin/sh

set -eu

here=$(cd "$(dirname "$0")" && pwd)
salam=${SALAM:-salam}
cc=${CC:-clang}
out=${OUT:-$here/kernel.elf}

cd "$here"

"$cc" --target=i686-unknown-none-elf -c boot.s -o boot.o

"$salam" build main.salam \
    --target=i686-unknown-none \
    --entry=_start \
    --linker-script=linker.ld \
    --link-object=boot.o \
    --output="$out" \
    --log-level=error

echo "kernel: $out"
