#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <model_name> <batch_size>"
    exit 1
fi

# Assign the arguments to variables
model_name=$1
batch_size=$2

# Construct the path to the input file
input_file="$model_name/cudnn$batch_size.txt"

# Initialize variable to store the total
total=0

# Open kernel_times.txt for writing
> kernel_times.txt

# Read each line of the file
while IFS=' ' read -r _ number _; do
    multiplied_number=$(awk "BEGIN {print $number * 1000}")
    # Add the number from the second column to the total
    total=$(awk "BEGIN {print $total + $multiplied_number}")
    # Append the total to kernel_times.txt
    echo "$total" >> kernel_times.txt
done < "$input_file"