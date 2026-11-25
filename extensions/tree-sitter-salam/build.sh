#!/bin/sh
# Generates the parser from grammar.js, runs the corpus tests and checks that
# every query in queries/ compiles against the grammar.
#
# Usage: extensions/tree-sitter-salam/build.sh [--sweep]
#
# --sweep also parses every valid .salam file in the repository (everything
# outside an errors/ directory), prints how many parse without an error, and
# fails if std/gui/webview or the stdcall tests do not parse.
#
# Env: TREE_SITTER  the tree-sitter CLI to use (default: tree-sitter on PATH)

set -eu

HERE=$(cd "$(dirname "$0")" && pwd)
TS=${TREE_SITTER:-tree-sitter}
cd "$HERE"
"$TS" generate
"$TS" test
for q in queries/*.scm; do
    "$TS" query "$q" test/queries/sample.salam >/dev/null
done
if [ "${1:-}" = "--sweep" ]; then
    mkdir -p build
    find ../../tests ../../std ../../compiler -name '*.salam' | grep -v '/errors/' | sort >build/files.txt
    "$TS" parse --quiet --stat --paths build/files.txt | grep "^Total" || true
    "$TS" parse --quiet ../../std/gui/webview/*.salam ../../tests/en/llvm/stdcall_fn_pointer.salam \
        ../../tests/fa/llvm/stdcall_fn_pointer.salam
fi
