#!/bin/bash

# Check if all required arguments are provided
if [ $# -ne 3 ]; then
    echo "Usage: $0 <model_name> <batch_size> <bandwidth>"
    exit 1
fi

# Extract arguments
WORKLOAD="$1"
BATCH="$2"
SSD="$3"

# Function to process a model
first_iter() {
    local model="$1"
    local batch_size="$2"
    local bandwidth="$3"

    # Change directory to the specified model directory

    # Get the 3rd word from the line including "iter1.exe_time" in sim_result.final
    if [ "$bandwidth" = "6_4" ]; then
        awk '/tensor/ {print $9}' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/statistics/tensors.config > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/it1-$batch_size-$bandwidth-tensor_memory.txt
        awk '/tensor/ {print $9}' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/statistics/tensors.config |
        awk '{printf "%s", $1}'  > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/it1-$batch_size-$bandwidth-tensor_memory_figs.txt
    elif [ "$bandwidth" = "64" ]; then
        awk '/tensor/ {print $9}' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie64/statistics/tensors.config > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie64/it1-$batch_size-$bandwidth-tensor_memory.txt
        awk '/tensor/ {print $9}' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie64/statistics/tensors.config |
        awk '{printf "%s", $1}' > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie64/it1-$batch_size-$bandwidth-tensor_memory_figs.txt
    elif [ "$bandwidth" = "128" ]; then
        awk '/tensor/ {print $9}' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie128/statistics/tensors.config > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie128/it1-$batch_size-$bandwidth-tensor_memory.txt
        awk '/tensor/ {print $9}' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie128/statistics/tensors.config |
        awk '{printf "%s", $1}' > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie128/it1-$batch_size-$bandwidth-tensor_memory_figs.txt
    else
        awk '/tensor/ {print $9}' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/statistics/tensors.config > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/it1-$batch_size-$bandwidth-tensor_memory.txt
        awk '/tensor/ {print $9}' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/statistics/tensors.config |
        awk '{printf "%s", $1}' > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/it1-$batch_size-$bandwidth-tensor_memory_figs.txt
    fi
    # Print the variables
    echo "Model: $model Batch: $batch_size SSD: $bandwidth"
}

#WORKLOAD="$1"
#BATCH="$2"
#SSD="$3"

first_iter $WORKLOAD $BATCH $SSD

# Process BERT_Base model
#process_model "BERT_Base"

# Process ViT model
#process_model "VIT"