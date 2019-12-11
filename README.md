# Multi-Dimentional Learned Index for Range Queries

## Dependencies
1.  boost: Ubuntu install `sudo apt-get install libboost-all-dev`
2.  libspatialindex: I haven't installed this on ubuntu, only MacOS. It contains the RTree implementation.

## Datasets
1.  Random: uniform random distribution, random query ranges
2.  Normal: normal distribution, query ranges are large
3.  Mix-Gauss: mixture Gaussian distribution, query ranges are small (many empty ranges)

## TODO
1.  ✔ Lookups (Adam)
    1.  ✔ Point
    2.  ✔ Range
2.  ✔ Building Tree (Adam)
3.  ✔ Stats on the tree and query performance (Adam)
4.  PCA Projection (Adam)
5.  Competitors (Xiyang)
    1.  R-Tree
    2.  KD-Tree
    3.  ✔ Full Scan
6.  Datasets (Xiyang)
    1.  Synthetic data
    2.  Real world data 
7.  Fanout model / Page size parameter picking/automating (Don't need to do this for course project)
