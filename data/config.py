from enum import Enum

# default dimension
DIM = 2
# default number of points
DATA_SIZE = 1000000
# default number of queries
QUERY_SIZE = 1000

# supported data distributions
class Distribution(Enum):

    RANDOM = 0
    NORMAL = 1
    MIX = 2
    
parseDistribution = {
    Distribution.RANDOM : "random",
    Distribution.NORMAL : "normal",
    Distribution.MIX : "mix-Gauss",
}