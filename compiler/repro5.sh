#!/bin/bash
cd /mnt/c/Users/Ali/Projects/SalamLang/Salam/compiler || exit
export SALAM_STD="$PWD/std"
echo "=== tcc version ==="; tcc -v 2>&1 | head -1
echo "=== force --cc=tcc, hello.salam, keep C, verbose ==="
rm -f /tmp/hello.exe salam_mod_*.c salam_mod_*.h
./salam build tests/en/general/hello.salam --output=/tmp/hello.exe --cc=tcc --no-color --log-level=info 2>&1 | head -40
echo "exit=${PIPESTATUS[0]}"
ls -la /tmp/hello.exe 2>&1
