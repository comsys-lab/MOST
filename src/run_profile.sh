#!/bin/bash

if [ $# -lt 2 ]; then
    echo "[ERROR] Usage: ./run_profile.sh <MODEL_NAME> <BATCH_SIZE 1> <BATCH_SIZE 2> ..."
    exit 1
fi

MODEL_NAME=$1
shift # Remove first argument

for BATCH_SIZE in "$@"; do
    echo "Running experiments for BATCH_SIZE: $BATCH_SIZE"
    ./gpg configs/$MODEL_NAME/${BATCH_SIZE}-profile.config
    #./gpg configs/$MODEL_NAME/${BATCH_SIZE}-profile-PF.config
    #./gpg configs/$MODEL_NAME/${$BATCH_SIZE}-profile-InputPF.config

    if [ "$MODEL_NAME" == "BERT" ]; then
    MODEL_NAME="BERT_Base"
    fi
    cp $MODEL_NAME/profile-${BATCH_SIZE}-cudnn/cudnn_kernel_times.txt ../results/$MODEL_NAME/cudnn$BATCH_SIZE.txt
    cp $MODEL_NAME/profile-${BATCH_SIZE}-cudnn/cudnn_kernel_times.txt ../results/$MODEL_NAME/cudnn${BATCH_SIZE}PF.txt
    cp $MODEL_NAME/profile-${BATCH_SIZE}-cudnn/cudnn_kernel_times.txt ../results/$MODEL_NAME/cudnn${BATCH_SIZE}InputPF.txt
    #cp $MODEL_NAME/profile-${BATCH_SIZE}-cudnn-PF/cudnn_kernel_times.txt ../results/$MODEL_NAME/cudnn${BATCH_SIZE}PF.txt
    #cp $MODEL_NAME/profile-${BATCH_SIZE}-cudnn-InputPF/cudnn_kernel_times.txt ../results/$MODEL_NAME/cudnn${BATCH_SIZE}InputPF.txt
    cp $MODEL_NAME/profile-${BATCH_SIZE}-cudnn/cudnn_workspace_sizes.txt ../results/$MODEL_NAME/cudnn${BATCH_SIZE}Workspace.txt

    if [ "$MODEL_NAME" == "BERT_Base" ]; then
    MODEL_NAME="BERT"
    fi

done

# ./gpg configs/ResNet152/cudnn-64.config
# ./gpg configs/ResNet152/cudnn-64PF.config
# ./gpg configs/ResNet152/cudnn-128.config
# ./gpg configs/ResNet152/cudnn-128PF.config
# ./gpg configs/ResNet152/cudnn-256.config
# ./gpg configs/ResNet152/cudnn-256InputPF.config
# # ./gpg configs/ResNet152/cudnn-512.config
# ./gpg configs/ResNet152/cudnn-512InputPF.config
# # ./gpg configs/ResNet152/cudnn-1024.config
# ./gpg configs/ResNet152/cudnn-1024InputPF.config
# ./gpg configs/ResNet152/cudnn-2048.config
# ./gpg configs/ResNet152/cudnn-2048InputPF.config

# ./gpg configs/Inceptionv3/cudnn-256.config
# ./gpg configs/Inceptionv3/cudnn-256PF.config
# ./gpg configs/Inceptionv3/cudnn-512.config
# ./gpg configs/Inceptionv3/cudnn-512PF.config
# ./gpg configs/Inceptionv3/cudnn-1024.config
# ./gpg configs/Inceptionv3/cudnn-1024PF.config
# ./gpg configs/Inceptionv3/cudnn-256InputPF.config
# ./gpg configs/Inceptionv3/cudnn-512InputPF.config
# ./gpg configs/Inceptionv3/cudnn-1024InputPF.config

# ./gpg configs/ResNeXt101_32x8d/cudnn-64.config
# ./gpg configs/ResNeXt101_32x8d/cudnn-64PF.config
# ./gpg configs/ResNeXt101_32x8d/cudnn-128.config
# ./gpg configs/ResNeXt101_32x8d/cudnn-128PF.config
# ./gpg configs/ResNeXt101_32x8d/cudnn-256.config
# ./gpg configs/ResNeXt101_32x8d/cudnn-256PF.config
# ./gpg configs/ResNeXt101_32x8d/cudnn-512.config
# ./gpg configs/ResNeXt101_32x8d/cudnn-512PF.config
# ./gpg configs/ResNeXt101_32x8d/cudnn-1024.config
# ./gpg configs/ResNeXt101_32x8d/cudnn-1024PF.config

# ./gpg configs/ResNeXt101_32x8d/cudnn-256InputPF.config
# ./gpg configs/ResNeXt101_32x8d/cudnn-512InputPF.config
# ./gpg configs/ResNeXt101_32x8d/cudnn-1024InputPF.config

# ./gpg configs/SENet154/cudnn-64.config
# ./gpg configs/SENet154/cudnn-64PF.config
# ./gpg configs/SENet154/cudnn-128.config
# ./gpg configs/SENet154/cudnn-128PF.config
# ./gpg configs/SENet154/cudnn-256.config
# ./gpg configs/SENet154/cudnn-256PF.config
# ./gpg configs/SENet154/cudnn-512.config
# ./gpg configs/SENet154/cudnn-512PF.config
# ./gpg configs/SENet154/cudnn-1024.config
# ./gpg configs/SENet154/cudnn-1024PF.config

# ./gpg configs/SENet154/cudnn-256InputPF.config
# ./gpg configs/SENet154/cudnn-512InputPF.config
# ./gpg configs/SENet154/cudnn-1024InputPF.config


# # ./gpg configs/WResNet101/cudnn-64.config
# # ./gpg configs/WResNet101/cudnn-64PF.config
# # ./gpg configs/WResNet101/cudnn-128.config
# # ./gpg configs/WResNet101/cudnn-128PF.config
# # ./gpg configs/WResNet101/cudnn-256.config
# # ./gpg configs/WResNet101/cudnn-256PF.config
# # ./gpg configs/WResNet101/cudnn-512.config
# # ./gpg configs/WResNet101/cudnn-512PF.config
# # ./gpg configs/WResNet101/cudnn-1024.config
# # ./gpg configs/WResNet101/cudnn-1024PF.config
# # ./gpg configs/WResNet101/cudnn-2048.config
# # ./gpg configs/WResNet101/cudnn-2048PF.config

# ./gpg configs/WResNet101/cudnn-256InputPF.config
# ./gpg configs/WResNet101/cudnn-512InputPF.config
# ./gpg configs/WResNet101/cudnn-1024InputPF.config
# ./gpg configs/WResNet101/cudnn-2048InputPF.config


# ./gpg configs/Inceptionv3/cudnn-1024.config
# ./gpg configs/Inceptionv3/cudnn-1024PF.config
# ./gpg configs/Inceptionv3/cudnn-1024InputPF.config
# ./gpg configs/Inceptionv3/cudnn-1280.config
# ./gpg configs/Inceptionv3/cudnn-1280PF.config
# ./gpg configs/Inceptionv3/cudnn-1280InputPF.config
# ./gpg configs/Inceptionv3/cudnn-1536.config
# ./gpg configs/Inceptionv3/cudnn-1536PF.config
# ./gpg configs/Inceptionv3/cudnn-1536InputPF.config
# ./gpg configs/Inceptionv3/cudnn-1792.config
# ./gpg configs/Inceptionv3/cudnn-1792PF.config
# ./gpg configs/Inceptionv3/cudnn-1792InputPF.config


# ./gpg configs/BERT/cudnn-512.config
# ./gpg configs/BERT/cudnn-512PF.config
# ./gpg configs/BERT/cudnn-512InputPF.config


# ./gpg configs/BERT/cudnn-768.config
# ./gpg configs/BERT/cudnn-768PF.config
# ./gpg configs/BERT/cudnn-768InputPF.config


# ./gpg configs/BERT/cudnn-256.config
# ./gpg configs/BERT/cudnn-256PF.config
# ./gpg configs/BERT/cudnn-256InputPF.config

# ./gpg configs/VIT/cudnn-512.config
# ./gpg configs/VIT/cudnn-512PF.config
# ./gpg configs/VIT/cudnn-512InputPF.config

# ./gpg configs/VIT/cudnn-1024.config
# ./gpg configs/VIT/cudnn-1024PF.config
# ./gpg configs/VIT/cudnn-1024InputPF.config

# ./gpg configs/VIT/cudnn-1536.config
# ./gpg configs/VIT/cudnn-1536PF.config
# ./gpg configs/VIT/cudnn-1536InputPF.config
