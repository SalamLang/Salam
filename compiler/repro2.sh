#!/bin/bash
cd /mnt/c/Users/Ali/Projects/SalamLang/Salam/compiler || exit
export SALAM=./salam
LANGS=en bash tools/bash/run-tests.sh general 2>&1 | tail -30
