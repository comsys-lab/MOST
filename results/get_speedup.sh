#!/bin/bash

# Check if all required arguments are provided
if [ $# -ne 2 ]; then
    echo "Usage: $0 <model_name> <batch_size>"
    exit 1
fi

# Extract arguments
WORKLOAD="$1"
BATCH="$2"
# SSD="$3"

# Function to process a model
process_model() {
    local model="$1"
    local batch_size="$2"
    local bandwidth="$3"

    # Change directory to the specified model directory

    # Get the 3rd word from the line including "iter1.exe_time" in sim_result.final
    if [ "$bandwidth" = "6_4" ]; then
        variable1=$(grep "iter1.exe_time" "$model/$batch_size-lru-ssd$bandwidth-pcie32/sim_result.final" | awk '{print $3}')
        variable2=$(grep "iter1.exe_time" "$model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/sim_result.final" | awk '{print $3}')

    elif [ "$bandwidth" = "128" ]; then
        variable1=$(grep "iter1.exe_time" "$model/$batch_size-lru-ssd$bandwidth-pcie128/sim_result.final" | awk '{print $3}')
        variable2=$(grep "iter1.exe_time" "$model/$batch_size-prefetch_lru-ssd$bandwidth-pcie128/sim_result.final" | awk '{print $3}')
    else
        variable1=$(grep "iter1.exe_time" "$model/$batch_size-lru-ssd$bandwidth-pcie32/sim_result.final" | awk '{print $3}')
        variable2=$(grep "iter1.exe_time" "$model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/sim_result.final" | awk '{print $3}')
    fi
    # Print the variables
    echo "Model: $model Batch: $batch_size SSD: $bandwidth"
    #echo "Speedup: $((variable1/$variable2))" | bc -l
    echo "scale=2; $variable1 / $variable2" | bc
    echo "lru: $variable1"
    echo "prefetch: $variable2"
}

#WORKLOAD="$1"
#BATCH="$2"
#SSD="$3"

if [ "$WORKLOAD" = "all" ]; then
    process_model "BERT_Base" "512" "128"
    process_model "VIT" "512" "128"
    process_model "BERT_Base" "512" "6_4"
    process_model "VIT" "512" "6_4"
    

else
    process_model $WORKLOAD $BATCH 6_4
    process_model $WORKLOAD $BATCH 32
    process_model $WORKLOAD $BATCH 128
fi
# Process BERT_Base model
#process_model "BERT_Base"

# Process ViT model
#process_model "VIT"