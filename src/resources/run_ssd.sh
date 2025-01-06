#!/bin/bash

rm ../logfile.txt
#./run.sh -p "(BERT\/256)-sim_(prefetch_lru)-pcie(6_4)-ssd(6_4)-.*\.config" -dr -j 6
#./run.sh -p "(BERT\/256|BERT\/512)-sim_(prefetch_lru)-ssd(6_4|12_8|25_6|128)-.*\.config" -dr -j 6
#./run.sh -p "(ResNet152\/512|ResNet152\/1024|BERT\/256|BERT\/512|VIT\/2048|VIT\/4096)-sim_(prefetch_lru)-ssd(6_4|12_8|25_6|128)-.*\.config" -dr -j 6
./run.sh -p "(BERT\/1024)-sim_(prefetch_lru)-ssd(12_8|25_6|128)-.*\.config" -dr -j 6
#./run.sh -p "(BERT\/256|BERT\/512)-sim_(prefetch_lru)-ssd(6_4|12_8|25_6|32|128)-.*\.config" -dr -j 6
#./run.sh -p "(BERT\/512)-sim_(prefetch_lru)-ssd(6_4)-.*\.config" -dr