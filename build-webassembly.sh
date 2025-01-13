#!/bin/bash

# Check for emcc installation
if ! command -v emcc >/dev/null 2>&1; then
	echo 'Error: emcc is not installed.' >&2
	echo 'Install from https://emscripten.org/docs/getting_started/downloads.html'
	exit 1
fi

OUTPUT_BASE="salam-wa"
EDITOR_DIR="../Salam-Editor/"

MEMORY_FLAGS="-s ALLOW_MEMORY_GROWTH=1"
RUNTIME_FLAGS="-s EXIT_RUNTIME=0 -s NO_EXIT_RUNTIME=1"
COMMON_FLAGS="-s EXPORTED_FUNCTIONS=['_main'] -s TOTAL_STACK=8388608"

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
	"src/validator_style.c"
	"src/hashmap.c"
	"src/hashmap_custom.c"
	"src/array_custom.c"
	"src/lexer.c"
	"src/ast.c"
	"src/ast_layout.c"
	"src/ast_layout_style.c"
	"src/main.c"
)

DEBUG_FLAGS=""
if [[ "$1" == "debug" ]]; then
	echo "Debug mode enabled."
	DEBUG_FLAGS="-s VERBOSE=1 -s ASSERTIONS=2 -v"
else
	echo "Debug mode not enabled."
fi

echo "Compiling C files to WebAssembly..."
if emcc "${sources[@]}" -o "${OUTPUT_BASE}.html" \
	"${MEMORY_FLAGS}" \
	"${RUNTIME_FLAGS}" \
	"${COMMON_FLAGS}" \
	"${DEBUG_FLAGS}"; then
	echo "Compilation successful. Output files:"
	echo "  ${OUTPUT_BASE}.html"
	echo "  ${OUTPUT_BASE}.js"
	echo "  ${OUTPUT_BASE}.wasm"

	if command -v npx >/dev/null 2>&1; then
		echo "Transpiling JavaScript for older browsers..."
		if yes | npx esbuild "${OUTPUT_BASE}.js" --outfile="${OUTPUT_BASE}.transpiled.js" --minify=true --target=esnext; then
			mv "${OUTPUT_BASE}.transpiled.js" "${OUTPUT_BASE}.js"
		else
			echo "Warning: Babel transpiling failed. JavaScript was not transpiled."
		fi
	else
		echo "Warning: npx command not found. JavaScript will not be transpiled for older browsers."
	fi

	sed -i ':a;N;$!ba;s/\n/\\n/g' "${OUTPUT_BASE}.js"

	if [ -d "$EDITOR_DIR" ]; then
		echo "Copying output files to $EDITOR_DIR"
		cp "${OUTPUT_BASE}.html" "$EDITOR_DIR"
		cp "${OUTPUT_BASE}.js" "$EDITOR_DIR"
		cp "${OUTPUT_BASE}.wasm" "$EDITOR_DIR"
		echo "Files copied successfully."
	else
		echo "Directory $EDITOR_DIR does not exist. Skipping copy."
	fi
else
	echo "Compilation failed."
	exit 1
fi
