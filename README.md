tm-experiments
==============

Transactional memory (mostly Intel® TSX) experiments

histogram
---------
Based on [Performance Evaluation of Intel® TSX for
HPC](http://pcl.intel-research.net/publications/SC13-TSX.pdf).

Image histogram construction workload. Concurrent threads directly update the
shared histogram. Such task comprises the core compute of many HPC workloads.

Building:

    CFLAGS=-D_USE_TSX make

Usage:

    ./bin/hist ./content/bmp_24.bmp
