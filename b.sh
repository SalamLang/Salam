#!/bin/bash

# Clear screen
cls
clear

# Delete file
if [ -e "s" ]; then
    rm s
fi

# Compile sallam.c
gcc -g -ggdb -o s sallam.c
# gcc -g -fsanitize=undefined,address -Walloca -o s sallam.c -lefence

# Check if compilation was successful
if [ $? -eq 0 ]; then
    # ./s input.sallam
    LSAN_OPTIONS=verbosity=1:log_threads=1 ./s input.sallam
    # ASAN_OPTIONS=detect_leaks=1 ./s input.sallam
    # LSAN_OPTIONS=verbosity=1:log_threads=1:detect_leaks=1 ./s input.sallam
else
    echo "Compilation failed!"
fi
