#!/bin/bash

# Set the output executable name
output="salam"

# List all source files
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
	"src/hashmap.c"
	"src/hashmap_custom.c"
	"src/array_custom.c"
	"src/lexer.c"
	"src/ast.c"
	"src/ast_layout.c"
	"src/ast_layout_style.c"
	"src/main.c"
)

# Compile each source file into an object file
for src in "${sources[@]}"; do
	echo "Compiling $src..."
	gcc -c "$src" -o "${src%.c}.o"
	if [ $? -ne 0 ]; then
		echo "Error: Compilation failed for $src"
		exit 1
	fi
done

# Link all object files into the final executable
echo "Linking object files..."
gcc -o "$output" *.o
if [ $? -ne 0 ]; then
	echo "Error: Linking failed"
	exit 1
fi

# Clean up object files (optional)
# rm *.o

# Run the executable with the provided argument
echo "Running the executable..."
./"$output" ../example/test6.salam ../out/
if [ $? -ne 0 ]; then
	echo "Error: Execution failed"
	exit 1
fi

echo "Build and execution completed successfully."
