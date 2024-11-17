#!/bin/bash

if ! command -v emcc >/dev/null 2>&1; then
	echo 'Error: emcc is not installed.' >&2
	echo 'Install from https://emscripten.org/docs/getting_started/downloads.html'
	exit 1
fi

OUTPUT_BASE="salam-wa"

sources=(
	"src/log.c"
	"src/file.c"
	"src/memory.c"
	"src/array.c"
	"src/parser.c"
	"src/parser_layout.c"
	"src/generator.c"
	"src/generator_layout.c"
	"src/downloader.c"
	"src/generator_salam.c"
	"src/generator_layout_style.c"
	"src/generator_identifier.c"
	"src/string_buffer.c"
	"src/validator.c"
	"src/hashmap.c"
	"src/hashmap_custom.c"
	"src/array_custom.c"
	"src/lexer.c"
	"src/ast.c"
	"src/ast_layout.c"
	"src/ast_layout_style.c"
	"src/main.c"
)

echo "Compiling C files to WebAssembly..."
emcc "${sources[@]}" -o ${OUTPUT_BASE}.html \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s EXIT_RUNTIME=1 \
	-s NO_EXIT_RUNTIME=1 \
	-s VERBOSE=1 \
	# -s "EXPORTED_FUNCTIONS=['_main']"
	# -s ASSERTIONS=0 \

if [ $? -eq 0 ]; then
	echo "Compilation successful. Output files:"
	echo "  ${OUTPUT_BASE}.html"
	echo "  ${OUTPUT_BASE}.js"
	echo "  ${OUTPUT_BASE}.wasm"
else
	echo "Compilation failed."
	exit 1
fi
