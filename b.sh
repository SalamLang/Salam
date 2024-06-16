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

ldconfig -p | grep efence &>/dev/null
if ! [ $? -eq 0 ]; then
	echo "efence library is missing"
	echo "Install efence - Electric Fence Malloc Debugger"
	exit 1
fi

# Compile
# gcc -g -ggdb -g -o "$OUTPUT_FILE" "$INPUT_FILE"
# gcc -g -fsanitize=undefined,address -Walloca -o "$OUTPUT_FILE" "$INPUT_FILE" -lefence
gcc -g -fsanitize=undefined,address -Walloca -o "$OUTPUT_FILE" "$INPUT_FILE"

# if [ $? -eq 0 ]; then
# 	if ! [ -x "$(command -v emcc)" ]; then
# 		echo 'Error: emcc is not installed.' >&2
# 		echo 'Install from https://emscripten.org/docs/getting_started/downloads.html'
# 		exit 1
# 	fi

# 	# Compiling for web
# 	emcc salam.c -o salam.js -s ALLOW_MEMORY_GROWTH=1 -s EXIT_RUNTIME=1 -s NO_EXIT_RUNTIME=1
# fi

# Check if compilation was successful
if [ $? -eq 0 ]; then
	./"$OUTPUT_FILE" "$EXAMPLE_FILE"
	# LSAN_OPTIONS=verbosity=1:log_threads=1 ./"$OUTPUT_FILE" "$EXAMPLE_FILE"
	# ASAN_OPTIONS=detect_leaks=1 ./"$OUTPUT_FILE" "$EXAMPLE_FILE"
	# LSAN_OPTIONS=verbosity=1:log_threads=1:detect_leaks=1 ./"$OUTPUT_FILE" "$EXAMPLE_FILE"
else
	echo "Compilation failed!"
fi

# ./s --code "تابع سلام {\n    نمایش \"سلام، دنیا\"}"

# ./s --code "تابع سلام() {
#     نمایش \"سلام، دنیا\"
# }"
