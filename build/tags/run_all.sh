#!/bin/bash

for script in *.py; do
    echo "Running $script..."
    # python3 "$script" || python "$script" || echo "Error running $script"
    python3 "$script" || echo "Error running $script"
done

echo "All scripts executed."
