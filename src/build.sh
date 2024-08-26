#!/bin/bash

if ! command -v emcc >/dev/null 2>&1; then
	echo 'Error: emcc is not installed.' >&2
	echo 'Install from https://emscripten.org/docs/getting_started/downloads.html'
	exit 1
fi

OUTPUT_BASE="output"

echo "Compiling C files to WebAssembly..."
emcc *.c -o ${OUTPUT_BASE}.html -s ALLOW_MEMORY_GROWTH=1 -s EXIT_RUNTIME=1 -s NO_EXIT_RUNTIME=1

if [ $? -eq 0 ]; then
	echo "Compilation successful. Output files:"
	echo "  ${OUTPUT_BASE}.html"
	echo "  ${OUTPUT_BASE}.js"
	echo "  ${OUTPUT_BASE}.wasm"
else
	echo "Compilation failed."
	exit 1
fi
