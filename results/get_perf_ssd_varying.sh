#!/bin/bash

# Ensure the correct number of arguments are provided
if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <model> <policy> <batch> <ITER> "
    exit 1
fi

# Assign input arguments to variables
POLICY=$2
MODEL=$1
BATCH=$3
ITER=$4
OUTPUT_FILE="$MODEL"_"$POLICY"_"$BATCH"_"iter$ITER".txt

# Base directory containing the folders
BASE_DIR="./$MODEL"

# Initialize arrays to store batch sizes and stats
ssd_sizes=()
exe_times=()
ssd_pfs=()

TARGET_DIR=$(ls -d $MODEL/"$BATCH"-"$POLICY"-*)
# Iterate through each subdirectory matching the pattern
for dir in $TARGET_DIR; do
    # Extract the batch size and Model from the directory name
    BATCH_SIZE=$(basename "$dir" | cut -d'-' -f3)
    BATCH_SIZE=${BATCH_SIZE:3}
    echo "$BATCH_SIZE"
    if [[ -d "$dir" ]]; then
        
        sim_file="$dir/sim_result.final"
        echo "$sim_file"
        if [[ -f "$sim_file" ]]; then
            # Get the values for exe_time and ssd_pf
            if [ "$ITER" = "all" ]; then
                exe_time=$(grep "kernel_stat.total.exe_time" "$sim_file" | awk '{print $3}')
                ssd_pf=$(grep "kernel_stat.total.ssd_pf" "$sim_file" | awk '{print $3}')
                iter0_stall=$(grep "total_time_breakdown_stall.iter0" "$sim_file" | awk '{print $3}')
                iter1_stall=$(grep "total_time_breakdown_stall.iter1" "$sim_file" | awk '{print $3}')
                total_stall=$((iter0_stall + iter1_stall))
            fi

            if [ "$ITER" = "first" ]; then
                exe_time=$(grep "kernel_stat.iter1.exe_time" "$sim_file" | awk '{print $3}')
                ssd_pf=$(grep "kernel_stat.iter1.ssd_pf" "$sim_file" | awk '{print $3}')
                iter0_stall=$(grep "total_time_breakdown_stall.iter0" "$sim_file" | awk '{print $3}')
                iter1_stall=$(grep "total_time_breakdown_stall.iter1" "$sim_file" | awk '{print $3}')
                total_stall=$((iter1_stall))
            fi


            # Save the batch size and corresponding stats
            batch_sizes+=("$BATCH_SIZE")
            exe_times+=("$exe_time")
            ssd_pfs+=("$ssd_pf")
            total_stalls+=("$total_stall") 
        fi
    fi
done

# Create or overwrite the CSV file and write the header with batch sizes
{
    printf "BATCH|" 
    printf "%s|" "${batch_sizes[@]}"
    echo ""

    # Write the exe_time row
    printf "EXE_TIME|" 
    printf "%s|" "${exe_times[@]}"
    echo ""

    # Write the ssd_pf row
    printf "SSD_PF|"
    printf "%s|" "${ssd_pfs[@]}"
    echo ""

    # Write the total stall row
    printf "STALL|"
    printf "%s|" "${total_stalls[@]}"
    echo ""
} > "$OUTPUT_FILE"

echo "Results saved to $OUTPUT_FILE"