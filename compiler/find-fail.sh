#!/bin/bash
cd /mnt/c/Users/Ali/Projects/SalamLang/Salam/compiler || exit 1
SALAM=./salam sh tools/bash/run-tests.sh 2>&1 | grep -E 'FAIL|RESULT'
