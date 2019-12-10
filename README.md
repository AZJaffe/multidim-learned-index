# Multi-Dimentional Learned Index for Range Queries

## Datasets
1.  Random: uniform random distribution, random query ranges
2.  Normal: normal distribution, query ranges are large
3.  Mix-Gauss: mixture Gaussian distribution, query ranges are small (many empty ranges)

## TODO
1.  ✔ Lookups (Adam)
    1.  ✔ Point
    2.  ✔ Range
2.  ✔ Building Tree (Adam)
3.  Fanout model / Page size parameter picking/automating
4.  PCA Projection
5.  Support partial range queries
6.  Competitors (Xiyang)
    1.  R-Tree
    2.  KD-Tree
    3.  ✔ Full Scan
7.  Datasets (Xiyang)
    1.  ✔ Synthetic data
    2.  Real world data