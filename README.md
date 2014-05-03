tm-experiments
==============

[![Build status](https://travis-ci.org/JIghtuse/tm-experiments.png?branch=master)](https://travis-ci.org/JIghtuse/tm-experiments)

Transactional memory (mostly Intel® TSX) experiments

histogram
---------
Based on [Performance Evaluation of Intel® TSX for
HPC](http://pcl.intel-research.net/publications/SC13-TSX.pdf).

Image histogram construction workload. Concurrent threads directly update the
shared histogram. Such task comprises the core compute of many HPC workloads.

Building:

    make -C histogram

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


haskell-example
---------------
Example taken from Simon Peyton Jones article in School of Haskell:
[Software Transactional Memory](https://www.fpcomplete.com/school/advanced-haskell/beautiful-concurrency/3-software-transactional-memory)

The same example as in Clojure, transfering some amount of money from one
account to another.

Building:

    ghc haskell-example/transfer.hs

Usage:

    haskell-example/transfer
