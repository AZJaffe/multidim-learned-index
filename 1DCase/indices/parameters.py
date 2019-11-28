from enum import Enum

class Parameter:

    def __init__(self, stages, cores, trainSteps, batchSizes, learningRates, keepRatios):
        self.stages = stages
        self.cores = cores
        self.trainSteps = trainSteps
        self.batchSizes = batchSizes
        self.learningRates = learningRates
        self.keepRatios = keepRatios


class ParameterPool(Enum):
    
    SEQUENCE = Parameter(stages = [1, 10],
                        cores = [[1, 8, 1], [1, 8, 1]],
                        trainSteps= [20000, 20000],
                        batchSizes = [50, 50],
                        learningRates = [0.0001, 0.0001],
                        keepRatios = [1.0, 1.0])
    
    RANDOM   = Parameter(stages = [1, 10],
                        cores = [[1, 8, 1], [1, 8, 1]],
                        trainSteps = [20000, 50000],
                        batchSizes = [50, 1000],
                        learningRates = [0.0001, 0.00001],
                        keepRatios = [1.0, 1.0])
    
    EXPONENTIAL  = Parameter(stages = [1, 10],
                        cores = [[1, 1], [1, 1]],
                        trainSteps = [10000, 2000000],
                        batchSizes = [50, 1000],
                        learningRates = [0.0001, 0.005],
                        keepRatios = [1.0, 1.0])