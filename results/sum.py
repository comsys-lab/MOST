
def summary(file_path):
    with open(file_path, "r") as file:
        # Initialize sum
        total = 0
        # Iterate over each line in the file
        for line in file:
            # Split the line by spaces and get the second element
            # Convert it to a float and add it to the total
            total += float(line.split()[1])

    return total

file_path_org = './BERT_Base/cudnn1024.txt'
file_path_PF = './BERT_Base/cudnn1024PF.txt'

sum_org = summary(file_path_org)
sum_PF = summary(file_path_PF)
diff = sum_PF - sum_org
bandwidth_base = 15.754
bandwidth_target = 128
print("org: {}, PF: {}".format(sum_org, sum_PF))
time_target = (bandwidth_base/bandwidth_target) * diff
print("normalized time", time_target*100/sum_org)

 

