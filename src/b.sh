#!/bin/bash

SALAM_OUTPUT=s

items=(
    # ------------- utility
    # log
    "utility/log/log_fatal/log_fatal.c"
    # memory
    "utility/memory/memory_free/memory_free.c"
    "utility/memory/memory_allocation/memory_allocation.c"
    # file
    "utility/file/file_reads/file_reads.c"

    # ------------- stage

    "main.c"
)

gcc -g -Wall -Wextra -pedantic -I$(pwd) "${items[@]}" -o "$SALAM_OUTPUT" 2> compile_warnings.txt

cat compile_warnings.txt

if [ $? -ne 0 ]; then
    echo "Compilation and linking failed!"
    exit 1
fi

echo "Running program with Valgrind to check for memory leaks and issues..."
valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes ./$SALAM_OUTPUT input.salam
