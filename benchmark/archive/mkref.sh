#!/bin/bash
cd "${1:?data dir}" || exit 1
for d in text binary; do
  gzip -6 -c $d.bin > $d.gz
  bzip2 -9 -c $d.bin > $d.bz2
  xz -6 -T1 -c $d.bin > $d.xz
  xz --format=lzma -6 -T1 -c $d.bin > $d.lzma
  zstd -3 -q -c $d.bin > $d.zst
  zstd -19 -q -c $d.bin > $d.19.zst
  rm -f $d.zip $d.7z $d.r5.rar
  7z a -tzip -mx=5 -bso0 -bsp0 $d.zip $d.bin
  7z a -t7z -mx=5 -bso0 -bsp0 $d.7z $d.bin
  rar a -ma5 -m3 -idq $d.r5.rar $d.bin
done
ls -l
