#!/bin/bash

# Check if the number of arguments is correct
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 MODEL_NAME BATCH_SIZE INPUTS"
    exit 1
fi

# Assign arguments to variables
MODEL_NAME="$1"
BATCH_SIZE="$2"
INPUTS="$3"

if [ "$MODEL_NAME" == "BERT" ]; then
    MODEL_NAME="BERT_Base"
fi


# Calculate the new batch size
BATCH_SIZE_NEW=$(( BATCH_SIZE * INPUTS ))

# Define the input and output file paths
INPUT_KERNEL_FILE="$MODEL_NAME/profile-${BATCH_SIZE}-cudnn/cudnn_kernel_times.txt"
INPUT_WORKSPACE_FILE="$MODEL_NAME/profile-${BATCH_SIZE}-cudnn/cudnn_workspace_sizes.txt"
OUTPUT_FILE_KERNEL="../results/$MODEL_NAME/cudnn$BATCH_SIZE_NEW.txt"
OUTPUT_FILE_KERNEL_PF="../results/$MODEL_NAME/cudnn${BATCH_SIZE_NEW}PF.txt"
OUTPUT_FILE_KERNEL_INPUTPF="../results/$MODEL_NAME/cudnn${BATCH_SIZE_NEW}InputPF.txt"
OUTPUT_FILE_WORKSPACE="../results/$MODEL_NAME/cudnn${BATCH_SIZE_NEW}Workspace.txt"

# Check if the input files exist
if [ ! -f "$INPUT_KERNEL_FILE" ] || [ ! -f "$INPUT_WORKSPACE_FILE" ]; then
    echo "Input files not found"
    exit 1
fi

# Multiply the second column of kernel times by inputs
awk -v inputs="$INPUTS" '{print $1, $2 * inputs, $3}' "$INPUT_KERNEL_FILE" > "$OUTPUT_FILE_KERNEL"
awk -v inputs="$INPUTS" '{print $1, $2 * inputs, $3}' "$INPUT_KERNEL_FILE" > "$OUTPUT_FILE_KERNEL_PF"
awk -v inputs="$INPUTS" '{print $1, $2 * inputs, $3}' "$INPUT_KERNEL_FILE" > "$OUTPUT_FILE_KERNEL_INPUTPF"
awk -v inputs="$INPUTS" '{print $1, $2 * inputs, $3}' "$INPUT_WORKSPACE_FILE" > "$OUTPUT_FILE_WORKSPACE"
exit 0