#!/bin/bash

# Check if model name argument is provided
if [ $# -ne 1 ]; then
    echo "Usage: $0 <model_name>"
    exit 1
fi

# Extract model name argument
model_name="$1"

# Change directory to the specified model directory
cd "$model_name" || { echo "Error: Model directory not found"; exit 1; }

# Create profiling/stalled_kernels directory if it doesn't exist
mkdir -p profiling/stalled_kernels

# Iterate through directories in the current model directory
for dir in */; do
    # Check if run.log file exists in the current directory
    if [ -f "${dir}run.log" ]; then
        # Extract directory name without trailing '/'
        dir_name=${dir%/}

        # Extract the desired information from run.log and save it to a text file
        output=$(grep "not yet arrived" "${dir}run.log" | awk '{print substr($2, 7)}')
        echo "$output" > "${dir_name}.txt"

        # Move the created text file to profiling/stalled_kernels directory
        mv "${dir_name}.txt" ../profiling/stalled_kernels/
    fi
done