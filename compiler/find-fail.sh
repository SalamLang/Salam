#!/bin/bash
cd /mnt/c/Users/Ali/Projects/SalamLang/Salam/compiler || exit 1
for i in 1 2 3 4 5; do
    echo "===== RUN $i ====="
    SALAM=./salam sh tools/bash/run-tests.sh 2>&1 | grep -E 'FAIL|RESULT'
done
