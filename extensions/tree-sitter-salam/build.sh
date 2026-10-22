#!/bin/sh
# Generates the parser from grammar.js and runs the corpus tests.
#
# Usage: extensions/tree-sitter-salam/build.sh [--sweep]
#
# --sweep also parses every valid .salam file in the repository (everything
# outside an errors/ directory) and prints how many parse without an error.
#
# Env: TREE_SITTER  the tree-sitter CLI to use (default: tree-sitter on PATH)

set -eu

HERE=$(cd "$(dirname "$0")" && pwd)
TS=${TREE_SITTER:-tree-sitter}
cd "$HERE"
"$TS" generate
"$TS" test
if [ "${1:-}" = "--sweep" ]; then
    mkdir -p build
    find ../../tests ../../std ../../compiler -name '*.salam' | grep -v '/errors/' | sort >build/files.txt
    "$TS" parse --quiet --stat --paths build/files.txt | grep "^Total" || true
fi
