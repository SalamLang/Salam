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
    gcc -c "$src" -o "${src%.c}.o"
done

# Link all object files into the final executable
gcc -o "$output" *.o

# Clean up object files (optional)
# rm *.o

# Run the executable with a provided argument
./"$output" ../example/test6.salam ../out/
