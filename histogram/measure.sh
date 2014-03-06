#!/bin/bash

for sz in $(seq 10000000 10000000 100000000);
do
  for method in "MUTEX" "ATOMIC" "TSX";
  do
    make clean
    CFLAGS="-D_USE_${method} -DFAKE_BMAP_SIZE=${sz}" make
    for i in $(seq 20);
    do
      ./bin/hist >> ./logs/${method}_${sz}.log
    done
  done
done
