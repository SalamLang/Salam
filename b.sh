#!/bin/bash

# Compile sallam.c
rm s
# gcc sallam.c -o s -fsanitize=address
# gcc sallam.c -o s -fsanitize=leak -llsan -Walloc-zero
# gcc -g -fsanitize=address -Walloca sallam.c -o s -lefence
# gcc -g -fsanitize=undefined,address -Walloca -o s sallam.c -lefence
gcc -g -Walloca sallam.c -o s -fsanitize=undefined,address
# gcc -g -ggdb -o s sallam.c

# Check if compilation was successful
if [ $? -eq 0 ]; then
    # If compilation was successful, execute the program with input.sallam
    # ./s input.sallam
    # ASAN_OPTIONS=detect_leaks=1 ./s input.sallam
    LSAN_OPTIONS=verbosity=1:log_threads=1:detect_leaks=1 ./s input.sallam
else
    # If compilation failed, display an error message
    echo "Compilation failed!"
fi
