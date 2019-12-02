from enum import Enum

################################################
# Configuration for 1D data
################################################
DATA_SIZE = 10000
PAGE_SIZE = 100

# distribution types we support
class Distribution(Enum):
    SEQUENCE = 0
    RANDOM = 1
    EXPONENTIAL = 2

parseDistribution = {
    Distribution.SEQUENCE: "sequence",
    Distribution.RANDOM: "random",
    Distribution.EXPONENTIAL: "exponential",
}


# path to sparse indexing files (with page)
sparseIdxFile = {
    Distribution.SEQUENCE: "data/sparse/s-sequence.csv",
    Distribution.RANDOM: "data/sparse/s-random.csv",
    Distribution.EXPONENTIAL: "data/sparse/s-exponential.csv",
}

# path to dense indexing files
denseIdxFile = {
    Distribution.SEQUENCE: "data/dense/d-sequence.csv",
    Distribution.RANDOM: "data/dense/d-random.csv",
    Distribution.EXPONENTIAL: "data/dense/d-exponential.csv",
}

dataDistributionFile = {
    Distribution.SEQUENCE: "data/sequence-distribution-plot",
    Distribution.RANDOM: "data/random-distribution-plot",
    Distribution.EXPONENTIAL: "data/exponential-distribution-plot",
}