#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR" || exit
export SALAM=./salam
LANGS=en bash tools/bash/run-tests.sh general 2>&1 | tail -30
