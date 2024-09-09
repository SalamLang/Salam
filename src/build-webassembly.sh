#!/bin/bash

if ! command -v emcc >/dev/null 2>&1; then
    echo 'Error: emcc is not installed.' >&2
    echo 'Install from https://emscripten.org/docs/getting_started/downloads.html'
    exit 1
fi

OUTPUT_BASE="salam-wa"

sources=(
    "log.c"
    "file.c"
    "memory.c"
    "array.c"
    "parser.c"
    "parser_layout.c"
    "generator.c"
    "generator_layout.c"
	"generator_salam.c"
    "generator_layout_style.c"
    "generator_identifier.c"
    "string_buffer.c"
    "validator.c"
    "hashmap.c"
    "hashmap_custom.c"
    "array_custom.c"
    "lexer.c"
    "ast.c"
    "ast_layout.c"
    "ast_layout_style.c"
    "main.c"
)

echo "Compiling C files to WebAssembly..."
emcc "${sources[@]}" -o ${OUTPUT_BASE}.html \
    -s ALLOW_MEMORY_GROWTH=1 \
	-s EXIT_RUNTIME=1 \
    -s NO_EXIT_RUNTIME=1 \
    # -s "EXPORTED_FUNCTIONS=['_main']"
	# -s ASSERTIONS=0 \
	# -s VERBOSE=1 \

if [ $? -eq 0 ]; then
    echo "Compilation successful. Output files:"
    echo "  ${OUTPUT_BASE}.html"
    echo "  ${OUTPUT_BASE}.js"
    echo "  ${OUTPUT_BASE}.wasm"
else
    echo "Compilation failed."
    exit 1
fi
