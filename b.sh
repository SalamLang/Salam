#!/bin/bash

# Variables
INPUT_FILE="salam.c"
OUTPUT_FILE="s"
EXAMPLE_FILE="input.salam"

# Check if an argument is provided
if [ "$#" -eq 1 ]; then
    EXAMPLE_FILE="$1"
fi

# Clear screen
cls
clear

# Delete file
if [ -e "$OUTPUT_FILE" ]; then
    rm "$OUTPUT_FILE"
fi

# Compile
# gcc -g -ggdb -o "$OUTPUT_FILE" "$INPUT_FILE"
gcc -g -fsanitize=undefined,address -Walloca -o "$OUTPUT_FILE" "$INPUT_FILE" -lefence

# Compile js
emcc salam.c -o salam.js -s ALLOW_MEMORY_GROWTH=1 -s EXIT_RUNTIME=1
 
# Check if compilation was successful
if [ $? -eq 0 ]; then
    ./"$OUTPUT_FILE" "$EXAMPLE_FILE"
    # LSAN_OPTIONS=verbosity=1:log_threads=1 ./"$OUTPUT_FILE" "$EXAMPLE_FILE"
    # ASAN_OPTIONS=detect_leaks=1 ./"$OUTPUT_FILE" "$EXAMPLE_FILE"
    # LSAN_OPTIONS=verbosity=1:log_threads=1:detect_leaks=1 ./"$OUTPUT_FILE" "$EXAMPLE_FILE"
else
    echo "Compilation failed!"
fi
