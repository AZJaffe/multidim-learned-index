import pandas as pd
import time, sys, os
import json

import btree as bt

sys.path.append(os.getcwd() + "/..")
import config

class BTreeEvaluator:

    def __init__(self, degree):
        self.degree = degree
    
    def _load_dense_data(self, distribution):
        data = pd.read_csv("../" + config.denseIdxFile[distribution])
        self.keys, self.values = [], []
        for i in range(data.shape[0]):
            self.keys.append(data.ix[i, 1])
            self.values.append(data.ix[i, 0])
    
    def _load_sparse_data(self, distribution):
        return

    def _dump_result(self, distribution, buildTime, averageSearchTime, averageCost):
        result = []
        for _, node in self.btree.nodes.items():
            items = {}
            for item in node.items:
                if item is None:
                    continue
                items[str(item.k)] = str(item.v)
            children = []
            for child in node.children:
                if child is None:
                    continue
                children.append(str(child))
            tmp = {"index": str(node.index),
                    "isLeaf": str(node.isLeaf),
                    "children": children,
                    "items": items,
                    "numOfKeys": str(node.numOfKeys)}
            result.append(tmp)
        
        modelPath = "../results/models/BTrees/" + config.parseDistribution[distribution] + "_dense.json"
        with open(modelPath, "w") as f:
            json.dump(result, f)
        
        performance = {"Build time": buildTime,
                        "Average search time": averageSearchTime,
                        "Average cost:": averageCost,
                        "Model size:": os.path.getsize(modelPath)}
        performancePath = "../results/performaces/BTrees/" + config.parseDistribution[distribution] + "_dense.json"
        with open(performancePath, "w") as f:
            json.dump(performance, f) 
        
    def evaluate_dense_distribution(self, distribution):
        self.btree = bt.BTree(self.degree)
        self._load_dense_data(distribution)
        print("Building")
        startTime = time.time()
        self.btree.build(self.keys, self.values)
        endTime = time.time()
        buildTime = endTime - startTime
        print("Build BTree Time: " + str(buildTime))
        print("Predicting")
        cost = 0
        startTime = time.time()
        for key in self.keys:
            searchResult = self.btree.search(bt.Item(key, -1))
            if not searchResult["found"]:
                print("Should never happen!")
                assert(1==0)
            cost += searchResult["itemCount"]
        endTime = time.time()
        averageSearchTime = (endTime - startTime) / len(self.keys)
        averageCost = cost / len(self.keys)
        print("Average search time: " + str(averageSearchTime))
        print("Average cost: " + str(averageCost))
        self._dump_result(distribution, buildTime, averageSearchTime, averageCost)
    
    def evaluate_sparse_distribution(self, distribution):
        return