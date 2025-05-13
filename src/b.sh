#!/bin/bash

clear

SALAM_OUTPUT=s

mapfile -t items < <(find . -type f -name "*.c")

gcc -g -Wall -Wextra -pedantic -I$(pwd) "${items[@]}" -o "$SALAM_OUTPUT" 2> compile_warnings.txt

cat compile_warnings.txt

if [ $? -ne 0 ]; then
    echo "Compilation and linking failed!"
    exit 1
fi

echo "Running program with Valgrind to check for memory leaks and issues..."
valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes ./$SALAM_OUTPUT input.salam

if [ -f tokens.json ]; then
    if command -v jq >/dev/null 2>&1; then
        echo "Beautifying tokens.json using jq..."
        jq . tokens.json > tokens.pretty.json && mv tokens.pretty.json tokens.json
        echo "Beautification complete."
    else
        echo "Warning: 'jq' is not installed. Skipping beautification."
        echo "To install: sudo apt install jq"
    fi
else
    echo "tokens.json not found. Skipping beautification."
fi
