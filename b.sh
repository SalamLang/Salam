#!/bin/bash

# Compile sallam.c
rm s
gcc sallam.c -o s

# Check if compilation was successful
if [ $? -eq 0 ]; then
    # If compilation was successful, execute the program with input.sallam
    ./s input.sallam
else
    # If compilation failed, display an error message
    echo "Compilation failed!"
fi
