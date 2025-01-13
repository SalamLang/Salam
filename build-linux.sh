#!/bin/bash

output="salam"

sources=(
	"src/log.c"
	"src/file.c"
	"src/memory.c"
	"src/array.c"
	"src/parser.c"
	"src/downloader.c"
	"src/parser_layout.c"
	"src/generator.c"
	"src/generator_layout.c"
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

for src in "${sources[@]}"; do
	echo "Compiling $src..."
	if ! gcc -c "$src" -o "${src%.c}.o"; then
		echo "Error: Compilation failed for $src"
		exit 1
	fi
done

echo "Linking object files..."
if ! gcc -o "$output" ./*.o; then
	echo "Error: Linking failed"
	exit 1
fi

# Clean up object files (optional)
# rm ./*.o


echo "Running the executable..."
if ! ./"$output" ../example/test6.salam ../out/; then
	echo "Error: Execution failed"
	exit 1
fi

echo "Build and execution completed successfully."
