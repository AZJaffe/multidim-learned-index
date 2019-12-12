# Multi-Dimentional Learned Index for Range Queries

## Dependencies
1.  boost: Ubuntu install `sudo apt-get install libboost-all-dev`
2.  libspatialindex: this library should be installed easily on MacOS. If you are running on Ubuntu, try following steps.
    1.  Download the source code from libspatialindex github
    2.  Run `cmake .`, `make`, `sudo make install` (By default this should install to path `/usr/local`)
    3.  During compile, link `/usr/local/lib/libspatialindex.so` manually
    4.  If you can compile but cannot execute, try `sudo ldconfig`

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
