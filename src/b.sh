#!/bin/bash

clear

SALAM_OUTPUT=s

items=(
    # ------------- utility
    # log
    "utility/log/log_info/log_info.c"
    "utility/log/log_fatal/log_fatal.c"
    "utility/log/log_error/log_error.c"
    # memory
    "utility/memory/memory_destroy/memory_destroy.c"
    "utility/memory/memory_allocation/memory_allocation.c"
    "utility/memory/memory_allocation_soft/memory_allocation_soft.c"
    "utility/memory/memory_calloc/memory_calloc.c"
    "utility/memory/memory_copy/memory_copy.c"
    "utility/memory/memory_move/memory_move.c"
    "utility/memory/memory_realloc/memory_realloc.c"
    # file
    "utility/file/file_reads/file_reads.c"
    "utility/file/file_exists/file_exists.c"
    "utility/file/file_write/file_write.c"
    "utility/file/file_delete/file_delete.c"
    "utility/file/file_append/file_append.c"
    "utility/file/file_create/file_create.c"
    "utility/file/file_size/file_size.c"
    # string
    "utility/string/string_hash/string_hash.c"
    "utility/string/string_length/string_length.c"
    "utility/string/string_compare/string_compare.c"
    "utility/string/string_duplicate/string_duplicate.c"
    # array
    "utility/array/array_append/array_append.c"
    "utility/array/array_create/array_create.c"
    "utility/array/array_delete/array_delete.c"
    "utility/array/array_free/array_free.c"
    "utility/array/array_get/array_get.c"
    "utility/array/array_prepend/array_prepend.c"
    "utility/array/array_resize/array_resize.c"
    "utility/array/array_size/array_size.c"
    # hashmap
    "utility/hashmap/hashmap_create/hashmap_create.c"
    "utility/hashmap/hashmap_free/hashmap_free.c"
    "utility/hashmap/hashmap_put/hashmap_put.c"
    "utility/hashmap/hashmap_remove/hashmap_remove.c"
    "utility/hashmap/hashmap_size/hashmap_size.c"
    # io
    "utility/io/io_directory/io_directory.c"

    # ------------- stage
    # scanner
    "stage/scanner/scanner_free/scanner_free.c"
    "stage/scanner/scanner_create/scanner_create.c"
    "stage/scanner/scanner_log/scanner_log.c"
    "stage/scanner/scanner_set/scanner_set.c"
    "stage/scanner/scanner_scan/scanner_scan.c"
    # token
    "stage/token/token_create/token_create.c"
    "stage/token/token_free/token_free.c"
    "stage/token/token_name/token_name.c"
    "stage/token/token_type/token_type.c"

    # ------------- entry point
    "main.c"
)

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
