#!/bin/bash
set -e

# clear

# ----------- Configuration -----------
# Choose: tcc or gcc
COMPILER="gcc"
SALAM_OUTPUT="s"
C_FILES="c_files.txt"

# CFLAGS_COMMON=""
CFLAGS_COMMON="-Wno-unused-parameter -g -Wall -Wextra -pedantic"
INCLUDE_DIRS="-I. -Isrc"

CFLAGS="$CFLAGS_COMMON $INCLUDE_DIRS"
LDFLAGS=""

OBJ_FILES=""

# ----------- Compilation -----------
if [[ "$COMPILER" == "tcc" ]]; then
	while IFS= read -r file; do
		echo "Compiling $file ..."
		if ! $COMPILER -c $CFLAGS "$file" -o "${file%.*}.o" 2>compile_error.tmp; then
			echo "Compilation failed for $file:"
			cat compile_error.tmp
			rm -f compile_error.tmp
			exit 1
		fi
		rm -f compile_error.tmp
		OBJ_FILES="$OBJ_FILES ${file%.*}.o"
	done <"$C_FILES"

	echo "Linking..."
	if ! $COMPILER -o "$SALAM_OUTPUT" $OBJ_FILES $LDFLAGS; then
		echo "Linking failed!"
		exit 1
	fi

elif [[ "$COMPILER" == "gcc" ]]; then
	echo "Compiling and linking with GCC..."
	if ! $COMPILER $CFLAGS @"$C_FILES" -o "$SALAM_OUTPUT" $LDFLAGS; then
		echo "Compilation or linking failed!"
		exit 1
	fi

else
	echo "Unknown compiler: $COMPILER"
	exit 1
fi

echo "Compilation and linking successful."

# ----------- Run Program -----------
echo "Running program..."
if ! ./"$SALAM_OUTPUT" input.salam; then
	echo "Program execution failed!"
	exit 1
fi
echo "Program executed successfully."
