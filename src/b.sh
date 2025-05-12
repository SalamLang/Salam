#!/bin/bash

gcc -g -Wall -Wextra -pedantic -o s main.c 2> compile_warnings.txt

cat compile_warnings.txt

if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

echo "Running program with Valgrind to check for memory leaks and issues..."

valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes ./s input.salam
