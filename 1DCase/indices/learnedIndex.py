import pandas as pd
import json

from parameters import *
import NN
import sys, os, gc, time
sys.path.append(os.getcwd() + "/..")
import config

# Current only implement 2 stages RMI
class LearnedIndex:

    def __init__(self):
        self.x = []
        self.y = []
        return

    def _load_data(self, distribution):
        data = pd.read_csv("../" + config.denseIdxFile[distribution])
        self.dataSize = data.shape[0]
        for i in range(self.dataSize):
            self.x.append(data.ix[i, 1])
            self.y.append(data.ix[i, 0])
    
    def __init_parameters(self, distribution):
        if distribution == config.Distribution.SEQUENCE:
            self.parameters = ParameterPool.SEQUENCE.value
        elif distribution == config.Distribution.RANDOM:
            self.parameters = ParameterPool.RANDOM.value
        elif distribution == config.Distribution.EXPONENTIAL:
            self.parameters = ParameterPool.EXPONENTIAL.value

    def _hybrid_train(self, distribution):
        numOfStages = len(self.parameters.stages)
        numOfSecondStage = self.parameters.stages[1]

        stageInputs = [[[] for i in range(numOfSecondStage)] for i in range(numOfStages)]
        stageLabels = [[[] for i in range(numOfSecondStage)] for i in range(numOfStages)]
        stageInputs[0][0] = self.x
        stageLabels[0][0] = self.y
        RMI = [[None for i in range(numOfSecondStage)] for i in range(numOfStages)]
        for i in range(0, numOfStages):
            for j in range(0, self.parameters.stages[i]):
                if len(stageLabels[i][j]) == 0:
                    continue
                inputs = stageInputs[i][j]
                labels = []
                # intialize label for stage 1
                if i == 0:
                    block_size = config.DATA_SIZE * 1.0 / numOfStages
                    for l in stageLabels[i][j]:
                        labels.append(int(l / block_size))
                else:
                    labels = stageLabels[i][j]
                model = NN.NeuralNet(distribution=distribution, x=inputs, y=labels,
                                    learningRate=self.parameters.learningRates[i],
                                    trainStepNum=self.parameters.trainSteps[i],
                                    keepRatio=self.parameters.keepRatios[i],
                                    batchSize=self.parameters.batchSizes[i],
                                    core=self.parameters.cores[i])
                model.train()
                #print("i: " + str(i) + " j: " + str(j))
                RMI[i][j] = NN.AbstrctNN(weights=model.get_weights(),
                                        bias=model.get_bias(),
                                        core=self.parameters.cores[i],
                                        err=model.get_err())
                del model
                gc.collect()
                # intialize x, y for stage 2
                if i < numOfStages - 1:
                    for k in range(len(stageInputs[i][j])):
                        #print("i: " + str(i) + " j: " + str(j))
                        predictStageIdx = RMI[i][j].predict(stageInputs[i][j][k])
                        if predictStageIdx > numOfSecondStage - 1:
                            predictStageIdx = numOfSecondStage - 1
                        stageInputs[i+1][predictStageIdx].append(stageInputs[i][j][k])
                        stageLabels[i+1][predictStageIdx].append(stageLabels[i][j][k])
        return RMI

    def train(self, distribution):
        self._load_data(distribution)
        self.__init_parameters(distribution)
        print("Start Train")
        startTime = time.time()
        self.learnedIndex = self._hybrid_train(distribution)
        endTime = time.time()
        learnTime = endTime - startTime
        print("Build Learned Index in: " + str(learnTime))
        self.evaluate(distribution)
    
    def _dump_result(self, learnTime, averageSearchTime, averageErr):
        resultStage1 = {}

    def evaluate(self, distribution):
        print("Evaluate")
        err = 0
        startTime = time.time()
        for i in range(len(self.x)):
            firstStagePred = self.learnedIndex[0][0].predict(self.x[i])
            if firstStagePred > self.parameters.stages[1] - 1:
                firstStagePred = self.parameters.stages[1] - 1
            secondStagePred = self.learnedIndex[1][firstStagePred].predict(self.x[i])
            err += abs(secondStagePred - self.y[i])
        endTime = time.time()
        averageSearchTime = (endTime - startTime) / len(self.x)
        print("Average Search Time: " + str(averageSearchTime))
        averageErr = err * 1.0 / len(self.x)
        print("Average Err: " + str(averageErr))
        


li = LearnedIndex()
li.train(config.Distribution.RANDOM)
#li.train(config.Distribution.EXPONENTIAL)