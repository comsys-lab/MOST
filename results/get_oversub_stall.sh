#!/bin/bash

# Check if model name argument is provided
if [ $# -ne 5 ]; then
    echo "Usage: $0 <model_name> $1 <batch_size> $2 <bandwidth> <policy> <iter>"
    exit 1
fi

# Extract model name argument
model_name="$1"
batch_size="$2"
bandwidth="$3"
policy="$4"
iter="$5"

if [ "$bandwidth" == "128" ]; then
    pcie="128"
else
    pcie="32"
fi
mkdir -p profiling/stalled_kernels
output_name="$model_name-$batch_size-ssd${bandwidth}-$policy"
output_kernel_name="$model_name-$batch_size-ssd${bandwidth}-kernelName"
# Change directory to the specified model directory

first_iter() {
    local model="$1"
    local batch_size="$2"
    local bandwidth="$3"
    local policy="$4"

    cd "$model_name/$batch_size-$policy-ssd${bandwidth}-pcie${pcie}" || { echo "Error: Model directory not found"; exit 1; }
    
    # Find kernel indices that run out of memory
    # output_mem=$(grep -n -x "0" "it1-$batch_size-$bandwidth-memory.txt" | cut -d: -f1 | while read -r line; do
    # echo $((line - 1))
    # done)
    output_mem=$(grep -n -x "0" "it1-$batch_size-$bandwidth-memory.txt" | cut -d: -f1)
    #output_mem=$(awk '{if ($1 < 8192) print NR}' "it1-$batch_size-$bandwidth-memory.txt")
    output_iter1=$(grep -E "^1\+" "sim_result.kernelStall")
    echo "$output_iter1" > sim_result.iterFirstKernel.txt

    output_iter1_stall=$(awk -F'[+:|=]' '{print $NF}' sim_result.iterFirstKernel.txt)
    echo "$output_iter1_stall" > "sim_result.iterFirstKernelStall.txt"
    # Find how much stalls for the indices
    
    echo "$output_mem" > "sim_result.iterFirstMemory.txt"

    sum=0

    # Loop through each row number from output_mem
    while read -r line_num; do
        echo "$line_num"
        line_num=$line_num+1
        value=$(awk "NR==$line_num" "sim_result.iterFirstKernelStall.txt")
        # Add the value to the sum
        sum=$((sum + value))
    done < sim_result.iterFirstMemory.txt

    # Output the total sum
    echo "Total sum: $sum"
    
}


# Create profiling/stalled_kernels directory if it doesn't exist

# output2=$(grep "Kernel ID" "$model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/statistics/kernels.config" | awk '{print $5}')
# output2=$(grep "Kernel ID" "statistics/kernels.config" | awk '{print $5}')
# echo "$output2" > "${output_kernel_name}.txt"

# output_iter1=$(grep -E "^1\+" "sim_result.kernelStall")
# echo "$output_iter1" > sim_result.iterKernelStall

# output=$(awk -F'[+:|=]' '{print $2, $NF}' sim_result.iterKernelStall)
# echo "$output" > "${output_name}.txt"
# paste -d ' ' ${output_name}.txt ${output_kernel_name}.txt > ${output_name}Merged.txt
# sort -k2,2nr "${output_name}Merged.txt" > "sorted_${output_name}.txt"

# output3=$(awk '
# {
#     sum[$3] += $2  # Accumulate the sum for each unique value in the third column
# }
# END {
#     for (val in sum) {
#         print val, sum[val]  # Print each unique third column value and its corresponding sum
#     }
# }' ${output_name}Merged.txt)
# echo "$output3" > "${output_name}BreakDown.txt"


# cp "${output_name}.txt" ../../profiling/stalled_kernels/
# cp "sorted_${output_name}.txt" ../../profiling/stalled_kernels/

if [ "$iter" = "all" ]; then
    all_iter $model_name $batch_size $bandwidth $policy

elif [ "$iter" = "first" ]; then
    first_iter $model_name $batch_size $bandwidth $policy
else
    echo "Usage: ITER should be all or first"
fi