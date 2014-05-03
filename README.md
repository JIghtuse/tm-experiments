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


clojure-example
---------------
Examples stolen from [sw1nn](https://github.com/sw1nn) blog: [Clojure STM -
What? Why? How?](http://sw1nn.com/blog/2012/04/11/clojure-stm-what-why-how/)

A few examples of Clojure STM, the single language which supports TM in the
core.

Usage:

    clojure clojure-example/transfer.clj
