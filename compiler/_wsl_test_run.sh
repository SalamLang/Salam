#!/bin/bash
set -e
cd /mnt/c/Users/Ali/Projects/SalamLang/Salam/compiler
make -j"$(nproc)" 2>&1 | tail -40
echo "=== BUILD DONE ==="
SALAM="$(pwd)/../salam" bash tools/bash/run-tests.sh 2>&1 | tail -300
