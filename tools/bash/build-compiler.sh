#!/bin/sh
# Quick build of salam.

set -e
. "$(dirname "$0")/lib.sh"
if [ -z "${CC:-}" ]; then
    if command -v gcc >/dev/null 2>&1; then
        CC=gcc
    elif command -v clang >/dev/null 2>&1; then
        CC=clang
    else
        CC=tcc
    fi
fi
case "$CC" in
*tcc*) LDLIBS="" ;;
*) LDLIBS="-lm" ;;
esac
# Globbed per directory, exactly like the Makefile's own $(SRCDIRS), and not
# spelled out file by file: a hardcoded list goes stale the moment a new .c
# lands, and the failure is an undefined-reference at link time naming a
# symbol whose file is right there on disk (src/semantic/sema_definit.c did
# this to the playground build). src/web is absent here for the same reason
# the Makefile leaves it out - it is the wasm entry surface, built only by
# build-wasm.sh.
SRC_DIRS="core source condcomp logger xml diag i18n langpack token lexer ast
        parser semantic codegen jsgen llvm interp layout fmt doc cli driver
        minify"
SRCS="c/src/salam.c"
for d in $SRC_DIRS; do SRCS="$SRCS c/src/$d/*.c"; done

# shellcheck disable=SC2086 # deliberately word-split and glob-expanded
"$CC" -Wall -Ic/src -o salam $SRCS $LDLIBS
echo "built ./salam with $CC"
