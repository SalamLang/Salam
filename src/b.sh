#!/bin/bash

# clear

# python3 generate_files.py
# python3 generate_base_all.py

SALAM_OUTPUT=s

# ────── Function to beautify JSON ──────
beautify_json_if_valid() {
    local file="$1"
    if [ -f "$file" ]; then
        if command -v jq >/dev/null 2>&1; then
            echo "Checking if $file is valid JSON..."
            if jq empty "$file" >/dev/null 2>&1; then
                echo "Beautifying $file using jq..."
                jq . "$file" > "${file}.pretty" && mv "${file}.pretty" "$file"
                echo "Beautification of $file complete."
            else
                echo "$file contains invalid JSON. Skipping beautification."
            fi
        else
            echo "Warning: 'jq' is not installed. Skipping beautification of $file."
            echo "To install: sudo apt install jq"
        fi
    else
        echo "$file not found. Skipping beautification."
    fi
}

# ────── Compilation ──────
# mapfile -t items < <(find . -type f -name "*.c")
# find . -type f -name "*.c" > c_files.txt
# gcc -Wno-unused-parameter -g -Wall -Wextra -pedantic -I$(pwd) "${items[@]}" -o "$SALAM_OUTPUT" 2> compile_warnings.txt

gcc -Wno-unused-parameter -g -Wall -Wextra -pedantic -I$(pwd) $(cat c_files.txt) -o "$SALAM_OUTPUT" 2> compile_warnings.txt

cat compile_warnings.txt

if [ $? -ne 0 ]; then
    echo "Compilation and linking failed!"
    exit 1
fi

# ────── Run the program ──────
echo "Running program with Valgrind to check for memory leaks and issues..."
valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes ./"$SALAM_OUTPUT" input.salam

VALGRIND_EXIT_CODE=$?
if [ $VALGRIND_EXIT_CODE -ne 0 ]; then
    echo "Program execution failed (exit code $VALGRIND_EXIT_CODE). Skipping JSON beautification."
    exit $VALGRIND_EXIT_CODE
fi

# ────── Beautify JSON outputs ──────
beautify_json_if_valid "tokens.json"
beautify_json_if_valid "ast.json"
