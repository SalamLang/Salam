#!/bin/bash

# Variables
INPUT_FILE="salam.c"
OUTPUT_FILE="s"
EXAMPLE_FILE="examples/input.salam"

# Check if an argument is provided
if [ "$#" -eq 1 ]; then
	EXAMPLE_FILE="$1"
fi

# Clear screen
clear

# Check if the virtual environment activation script exists
if [ -f /opt/venv/bin/activate ]; then
	# Activate the virtual environment
	source /opt/venv/bin/activate
	echo "Virtual environment activated."
else
	echo "Virtual environment not found."
fi

# Source Emscripten environment
if [ -f /opt/emsdk/emsdk_env.sh ]; then
	source /opt/emsdk/emsdk_env.sh
	echo "Emscripten environment activated."
else
	echo "Emscripten environment not found."
fi

# Delete file
if [ -e "$OUTPUT_FILE" ]; then
	rm "$OUTPUT_FILE"
fi

# Check if ldconfig command is available
if ! command -v ldconfig &>/dev/null; then
	echo "ldconfig command is not found. Please install ldconfig."
	exit 1
fi

ldconfig -p | grep efence &>/dev/null
if ! [ $? -eq 0 ]; then
	echo "efence library is missing"
	echo "Install efence - Electric Fence Malloc Debugger"
	echo ldconfig -p | grep efence
	exit 1
fi

echo "efence library is installed"

# Compile
echo "Compiling..."
# gcc -g -ggdb -g -o "$OUTPUT_FILE" "$INPUT_FILE"
# gcc -g -fsanitize=undefined,address -Walloca -o "$OUTPUT_FILE" "$INPUT_FILE" -lefence
gcc -g -fsanitize=undefined,address -Walloca -o "$OUTPUT_FILE" "$INPUT_FILE"
# gcc -o "$OUTPUT_FILE" "$INPUT_FILE" -Wall

if [ $? -eq 0 ]; then
	if ! [ -x "$(command -v emcc)" ]; then
		echo 'Error: emcc is not installed.' >&2
		echo 'Install from https://emscripten.org/docs/getting_started/downloads.html'
		exit 1
	fi

	# Compiling for web
	emcc "$INPUT_FILE" -o salam.js -s ALLOW_MEMORY_GROWTH=1 -s EXIT_RUNTIME=1 -s NO_EXIT_RUNTIME=1
fi

# Check if compilation was successful
echo "Running..."
if [ $? -eq 0 ]; then
	./"$OUTPUT_FILE" "$EXAMPLE_FILE"
else
	echo "Compilation failed!"
fi

# ./s --code "تابع سلام {\n    نمایش \"سلام، دنیا\"}"

# ./s input.salam
# ./s --code "تابع سلام() {
#     نمایش \"سلام، دنیا\"
# }"

if command -v pre-commit &>/dev/null; then
	echo "Running pre-commit..."
	pre-commit run --all-files
else
	echo "pre-commit is not installed."
fi
