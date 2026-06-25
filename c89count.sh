#!/bin/bash
cd /mnt/c/Users/Ali/Projects/SalamLang/Salam || exit 1
tot=0
files=0
for f in $(find src -name '*.c'); do
  n=$(gcc -Isrc -std=gnu89 -fsyntax-only "$f" 2>&1 | grep -c 'loop initial declarations')
  if [ "$n" -gt 0 ]; then files=$((files+1)); fi
  tot=$((tot+n))
done
echo "for-loop-decl errors (gnu89): $tot across $files files"
# also count under gnu89 the total hard errors (excluding warnings)
err=0
for f in $(find src -name '*.c'); do
  n=$(gcc -Isrc -std=gnu89 -fsyntax-only "$f" 2>&1 | grep -c 'error:')
  err=$((err+n))
done
echo "total hard errors (gnu89): $err"
