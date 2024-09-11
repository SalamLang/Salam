#!/bin/bash

# Set the output executable name
output="salam"

# List all source files
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
