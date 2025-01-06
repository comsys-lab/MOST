#!/bin/bash

# Check if model name argument is provided
if [ $# -ne 3 ]; then
    echo "Usage: $0 <model_name> $1 <batch_size> $2 <ssd>"
    exit 1
fi

# Extract model name argument
model_name="$1"
batch_size="$2"
bandwidth="$3"

if [ "$bandwidth" == "128" ]; then
    pcie="128"
else
    pcie="32"
fi
mkdir -p profiling/stalled_kernels
output_name="$model_name-$batch_size-ssd${bandwidth}"
output_kernel_name="$model_name-$batch_size-ssd${bandwidth}-kernelName"
# Change directory to the specified model directory
cd "$model_name/$batch_size-prefetch_lru-ssd${bandwidth}-pcie${pcie}" || { echo "Error: Model directory not found"; exit 1; }

# Create profiling/stalled_kernels directory if it doesn't exist

# output2=$(grep "Kernel ID" "$model/$batch_size-prefetch_lru-ssd$bandwidth-pcie32/statistics/kernels.config" | awk '{print $5}')
output2=$(grep "Kernel ID" "statistics/kernels.config" | awk '{print $5}')
echo "$output2" > "${output_kernel_name}.txt"

output_iter1=$(grep -E "^1\+" "sim_result.kernelStall")
echo "$output_iter1" > sim_result.iterKernelStall

output=$(awk -F'[+:|=]' '{print $2, $NF}' sim_result.iterKernelStall)
echo "$output" > "${output_name}.txt"
paste -d ' ' ${output_name}.txt ${output_kernel_name}.txt > ${output_name}Merged.txt
sort -k2,2nr "${output_name}Merged.txt" > "sorted_${output_name}.txt"

output3=$(awk '
{
    sum[$3] += $2  # Accumulate the sum for each unique value in the third column
}
END {
    for (val in sum) {
        print val, sum[val]  # Print each unique third column value and its corresponding sum
    }
}' ${output_name}Merged.txt)
echo "$output3" > "${output_name}BreakDown.txt"


cp "${output_name}.txt" ../../profiling/stalled_kernels/
cp "sorted_${output_name}.txt" ../../profiling/stalled_kernels/