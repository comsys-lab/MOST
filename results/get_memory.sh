#!/bin/bash

# Check if all required arguments are provided
if [ $# -ne 4 ]; then
    echo "Usage: $0 <model_name> <batch_size> <bandwidth> <iter>"
    exit 1
fi

# Extract arguments
WORKLOAD="$1"
BATCH="$2"
SSD="$3"
ITER="$4"

# Function to process a model
first_iter() {
    local model="$1"
    local batch_size="$2"
    local bandwidth="$3"

    # Change directory to the specified model directory

    # Get the 3rd word from the line including "iter1.exe_time" in sim_result.final
    if [ "$bandwidth" = "6_4" ]; then
        awk '/Kernel0/ && /ITER: 1/,0' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/run.log |
        grep -B 3 "Guide report" |
        awk -F'=' '/GPU/ {if (NF >= 2) print $2}' |
        awk -F'[</>]' '{print $2}' > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/it1-$batch_size-$bandwidth-memory.txt
        
        awk '{printf "%s,", $1}' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/it1-$batch_size-$bandwidth-memory.txt |
        sed 's/,$//'  > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/it1-$batch_size-$bandwidth-memory_figs.txt
    elif [ "$bandwidth" = "64" ]; then
        awk '/Kernel0/ && /ITER: 1/,0' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie64/run.log |
        grep -B 3 "Guide report" |
        awk -F'=' '/GPU/ {if (NF >= 2) print $2}' |
        awk -F'[</>]' '{print $2}' > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie64/it1-$batch_size-$bandwidth-memory.txt

        awk '{printf "%s,", $1}' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie64/it1-$batch_size-$bandwidth-memory.txt |
        sed 's/,$//'  > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie64/it1-$batch_size-$bandwidth-memory_figs.txt    
    elif [ "$bandwidth" = "128" ]; then
        awk '/Kernel0/ && /ITER: 1/,0' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie128/run.log |
        grep -B 3 "Guide report" |
        awk -F'=' '/GPU/ {if (NF >= 2) print $2}' |
        awk -F'[</>]' '{print $2}' > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie128/it1-$batch_size-$bandwidth-memory.txt

        awk '{printf "%s,", $1}' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie128/it1-$batch_size-$bandwidth-memory.txt |
        sed 's/,$//'  > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie128/it1-$batch_size-$bandwidth-memory_figs.txt
    else
        awk '/Kernel0/ && /ITER: 1/,0' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/run.log |
        grep -B 3 "Guide report" |
        awk -F'=' '/GPU/ {if (NF >= 2) print $2}' |
        awk -F'[</>]' '{print $2}' > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/it1-$batch_size-$bandwidth-memory.txt

        awk '{printf "%s,", $1}' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/it1-$batch_size-$bandwidth-memory.txt |
        sed 's/,$//'  > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/it1-$batch_size-$bandwidth-memory_figs.txt
    fi
    # Print the variables
    echo "Model: $model Batch: $batch_size SSD: $bandwidth"
}

all_iter() {
    local model="$1"
    local batch_size="$2"
    local bandwidth="$3"

    # Change directory to the specified model directory

    # Get the 3rd word from the line including "iter1.exe_time" in sim_result.final
    if [ "$bandwidth" = "6_4" ]; then
        awk '/Kernel0/,0' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/run.log |
        grep -B 3 "Guide report" |
        awk -F'=' '/GPU/ {if (NF >= 2) print $2}' |
        awk -F'[</>]' '{print $2}' > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/itAll-$batch_size-$bandwidth-memory.txt

        awk '{printf "%s,", $1}' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/itAll-$batch_size-$bandwidth-memory.txt |
        sed 's/,$//'  > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/itAll-$batch_size-$bandwidth-memory_figs.txt
    elif [ "$bandwidth" = "64" ]; then
        awk '/Kernel0/,0' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie64/run.log |
        grep -B 3 "Guide report" |
        awk -F'=' '/GPU/ {if (NF >= 2) print $2}' |
        awk -F'[</>]' '{print $2}' > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie64/itAll-$batch_size-$bandwidth-memory.txt

        awk '{printf "%s,", $1}' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie64/itAll-$batch_size-$bandwidth-memory.txt |
        sed 's/,$//'  > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie64/itAll-$batch_size-$bandwidth-memory_figs.txt    
    elif [ "$bandwidth" = "128" ]; then
        awk '/Kernel0/,0' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie128/run.log |
        grep -B 3 "Guide report" |
        awk -F'=' '/GPU/ {if (NF >= 2) print $2}' |
        awk -F'[</>]' '{print $2}' > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie128/itAll-$batch_size-$bandwidth-memory.txt

        awk '{printf "%s,", $1}' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie128/itAll-$batch_size-$bandwidth-memory.txt |
        sed 's/,$//'  > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie128/itAll-$batch_size-$bandwidth-memory_figs.txt
    else
        awk '/Kernel0/,0' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/run.log |
        grep -B 3 "Guide report" |
        awk -F'=' '/GPU/ {if (NF >= 2) print $2}' |
        awk -F'[</>]' '{print $2}' > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/itAll-$batch_size-$bandwidth-memory.txt

        awk '{printf "%s,", $1}' $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie128/itAll-$batch_size-$bandwidth-memory.txt |
        sed 's/,$//'  > $model/$batch_size-prefetch_lru-ssd$bandwidth-pcie128/itAll-$batch_size-$bandwidth-memory_figs.txt
    fi
    # Print the variables
    echo "Model: $model Batch: $batch_size SSD: $bandwidth"
}

#WORKLOAD="$1"
#BATCH="$2"
#SSD="$3"

if [ "$ITER" = "all" ]; then
    all_iter $WORKLOAD $BATCH $SSD

elif [ "$ITER" = "first" ]; then
    first_iter $WORKLOAD $BATCH $SSD
else
    echo "Usage: ITER should be all or first"
fi
# Process BERT_Base model
#process_model "BERT_Base"

# Process ViT model
#process_model "VIT"