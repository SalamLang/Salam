#!/bin/bash

# Variables
INPUT_FILE=salam.c
OUTPUT_FILE=s
EXAMPLE_FILE=input.salam

# Clear screen
cls
clear

# Delete file
if [ -e $OUTPUT_FILE ]; then
    rm $OUTPUT_FILE
fi

# Compile
# gcc -g -ggdb -o $OUTPUT_FILE $INPUT_FILE
gcc -g -fsanitize=undefined,address -Walloca -o s $INPUT_FILE -lefence

# Check if compilation was successful
if [ $? -eq 0 ]; then
    ./$OUTPUT_FILE $EXAMPLE_FILE
    # LSAN_OPTIONS=verbosity=1:log_threads=1 ./$OUTPUT_FILE $EXAMPLE_FILE
    # ASAN_OPTIONS=detect_leaks=1 ./$OUTPUT_FILE $EXAMPLE_FILE
    # LSAN_OPTIONS=verbosity=1:log_threads=1:detect_leaks=1 ./$OUTPUT_FILE $EXAMPLE_FILE
else
    echo "Compilation failed!"
fi
