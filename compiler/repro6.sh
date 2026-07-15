#!/bin/bash
cd /mnt/c/Users/Ali/Projects/SalamLang/Salam/compiler || exit
export SALAM_STD="$PWD/std"
rm -f /tmp/hello.exe salam_mod_*.c salam_mod_*.h /tmp/hello.c
./salam build tests/en/general/hello.salam --output=/tmp/hello.exe --cc=tcc --no-color --log-level=info --keep-c > /tmp/build.log 2>&1
echo "exit=$?"
echo "=== last 25 lines of build log ==="
tail -25 /tmp/build.log
echo "=== hello.exe? ==="; ls -la /tmp/hello.exe 2>&1
