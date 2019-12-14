import numpy as np
import csv, math
import os
from scipy.stats import norm, zipf
import matplotlib.pyplot as plt

from config import *

# sorting function
def _sort_keys(x):
        keys = []
        for i in range(DIM):
            keys.append(x[i])
        return keys

class Generator:

    def __init__(self):
        self.perColSelectivity = math.pow(SELECTIVITY, 1 / DIM)
        return
    
    def generate(self, distribution):
        points = None
        queries = None
        if distribution == Distribution.RANDOM:
            points = self._generate_random_points()
            queries = self._generate_random_queries()
        elif distribution == Distribution.NORMAL:
            points = self._generate_normal_points()
            queries = self._generate_normal_queries()
        elif distribution == Distribution.ZIPF:
            points = self._generate_zipf_points()
            queries = self._generate_zipf_queries()
        
        path = str(DIM) + "D-" + str(SELECTIVITY) + "S-" + parseDistribution[distribution]
        if not os.path.exists(path):
            os.makedirs(path)
        self._write_to_file(points, path + "/points.csv")
        self._write_to_file(queries, path + "/queries.csv")
        return

    def _generate_random_points(self):
        points = np.random.uniform(low=0, high=DATA_SIZE, size=[DATA_SIZE, DIM])
        points = np.array(sorted(points, key=_sort_keys))
        return points
    
    def _generate_random_queries(self):
        queries = []
        for i in range(QUERY_SIZE):
            query = []
            for j in range(DIM):
                start = np.random.uniform(low=0, high=((1-self.perColSelectivity) * DATA_SIZE))
                end = start + self.perColSelectivity * DATA_SIZE
                query.append(start)
                query.append(end)
            queries.append(query)
        return queries
    
    def _generate_normal_points(self):
        l = DATA_SIZE / 2
        s = DATA_SIZE / 5
        points = np.random.normal(loc=l, scale=s, size=[DATA_SIZE, DIM])
        points = np.array(sorted(points, key=_sort_keys))
        self.normal_min = np.min(points)
        points -= self.normal_min
        return points
    
    def _generate_normal_queries(self):
        l = DATA_SIZE / 2
        s = DATA_SIZE / 5
        queries = []
        for i in range(QUERY_SIZE):
            query = []
            for j in range(DIM):
                start = np.random.normal(loc=l, scale=s)
                while (norm.cdf(start, l, s) + self.perColSelectivity >= 1):
                    start = np.random.normal(loc=l, scale=s)
                end = norm.ppf(norm.cdf(start, l, s) + self.perColSelectivity, l , s)
                query.append(start - self.normal_min)
                query.append(end - self.normal_min)
            queries.append(query)
        return queries
    
    def _generate_zipf_points(self):
        a = 1.5
        points = np.random.zipf(a, size=[DATA_SIZE, DIM])
        points = np.array(sorted(points, key=_sort_keys))
        points = points.astype(float)
        noise = np.random.uniform(low=-EPSILON, high=EPSILON, size=[DATA_SIZE, DIM])
        points += noise 
        return points
    
    def _generate_zipf_queries(self):
        a = 1.5
        queries = []
        for i in range(QUERY_SIZE):
            query = []
            for j in range(DIM):
                start = np.random.zipf(a)
                while (zipf.cdf(start, a=a) + self.perColSelectivity >= 1):
                    start = np.random.zipf(a)
                end = zipf.ppf(zipf.cdf(start, a=a) + self.perColSelectivity, a=a)
                query.append(start)
                query.append(end)
            queries.append(query)
        return queries
    
    def _write_to_file(self, data, path):
        with open(path, "w") as f:
            csvWriter = csv.writer(f)
            for idx, row in enumerate(data):
                if idx % 50000 == 0:
                    print("Finish writing points: " + str(idx))
                csvWriter.writerow(row)
        return
    
    def _plot(self, data):
        x, y = data.T
        plt.scatter(x, y, s=10)
        plt.show()
        return

g = Generator()
g.generate(Distribution.RANDOM)
g.generate(Distribution.NORMAL)
g.generate(Distribution.ZIPF)