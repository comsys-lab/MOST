import os
import re
import argparse
import numpy as np

parser = argparse.ArgumentParser()
parser.add_argument("-m", help="model name", type=str, default='all')
parser.add_argument("-p", help="policy", type=str)
parser.add_argument("-base", help="base batch size", type=str)
parser.add_argument("-bound", help="bound batch size", type=str)
parser.add_argument("-ssd", help="SSD bandwidth", type=str, default='128')
args = parser.parse_args()

def extract_kernel_stat(file_path):
    kernel_stat_values = []
    with open(file_path, 'r') as file:
        for line in file:
            match = re.search(r'iter1.slowdown', line)
            
            if match:
                value = line.split("=")[1].strip()
                kernel_stat_values.append(float(value))
    return kernel_stat_values

def average_kernel_stat(directory):
    total_values = 0
    total_count = 0
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file == 'sim_result.final':
                file_path = os.path.join(root, file)
                kernel_stat_values = extract_kernel_stat(file_path)
                total_values += sum(kernel_stat_values)
                total_count += len(kernel_stat_values)
    if total_count == 0:
        return None
    else:
        return total_values / total_count

# Directory to start searching
start_directory = './'  # You can change this to the directory where you want to start the search
if args.m != 'all':
    start_directory += args.m
base_batch = int(args.base)
bound_batch = int(args.bound)
total_slowdown = []
count = 0
# Search for directories matching the pattern "*-ssd128-pcie128" and calculate the average kernel_stat
for root, dirs, files in os.walk(start_directory):
    for directory in dirs:
        
        if len(directory.split('-')) < 2:
            break
        
        batch_size = int(directory.split('-')[0])
        if args.ssd == '128':
            if re.match(str(batch_size)+'-'+args.p + '-ssd128-pcie128$', directory):
                if batch_size >= base_batch and batch_size <= bound_batch:
                    directory_path = os.path.join(root, directory)
                    print(directory_path)
                    total_slowdown.append(average_kernel_stat(directory_path))
                    print(average_kernel_stat(directory_path))
                    count += 1
        elif args.ssd == '6_4':
            if re.match(str(batch_size)+'-'+args.p + '-ssd6_4-pcie32$', directory):
                if batch_size >= base_batch and batch_size <= bound_batch:
                    directory_path = os.path.join(root, directory)
                    print(directory_path)
                    total_slowdown.append(average_kernel_stat(directory_path))
                    print(average_kernel_stat(directory_path))
                    count += 1
    
    
if count > 0:
    avg_slowdown = np.prod(total_slowdown)**(1.0/len(total_slowdown))
    #avg_slowdown = total_slowdown / count
    print(f"Average kernel_stat for {args.m} {args.p}: {avg_slowdown}")
else:
    print(f"No kernel_stat found in {args.m}")