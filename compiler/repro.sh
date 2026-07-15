#!/bin/bash
set -e
cd /mnt/c/Users/Ali/Projects/SalamLang/Salam/compiler
echo "=== compilers ==="
which tcc gcc clang cc 2>&1
echo "=== salam bin ==="
ls -la salam
echo "=== try build one test ==="
./salam build tests/en/general/arith.salam --output=/tmp/arith.exe --no-color --log-level=error --lang=en 2>&1 | head -40
echo "exit=$?"
ls -la /tmp/arith.exe 2>&1
