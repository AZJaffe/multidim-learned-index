import matplotlib.pyplot as plt
import numpy as np
import csv, math
import os

from config import *

class Generator:

    def __init__(self):
        return

    def generate(self, dim=DIM, distribution=Distribution.RANDOM, size=DATA_SIZE, qsize=QUERY_SIZE):
        # generate points
        points = None
        if distribution == Distribution.RANDOM:
            points = self._generate_random_points(dim, size)
        elif distribution == Distribution.NORMAL:
            points = self._generate_normal_points(dim, size)
        elif distribution == Distribution.MIX:
            points = self._generate_mixGauss_points(dim, size)
        else:
            print("Unsupported Distribution type")
            assert(1==0)
        # generate queries
        queries = self.generate_queries(dim=dim, distribution=distribution, qsize=qsize)
        # write
        path = str(dim) + "D-" + parseDistribution[distribution]
        if not os.path.exists(path):
            os.makedirs(path)
        self._write_to_file(points, path + "/points.csv")
        self._write_to_file(queries, path + "/queries.csv")
        # plot
        if dim == 2:
            self._plot(points, path + "/points-distribution")
        return

    def generate_queries(self, dim=DIM, distribution=Distribution.RANDOM, qsize=QUERY_SIZE):
        queries = []
        if distribution == Distribution.RANDOM:
            for _ in range(qsize):
                query = []
                for _ in range(dim):
                    start = np.random.uniform(low=0, high=DATA_SIZE)
                    selectivity = np.random.rand() * 0.1
                    end = start + (DATA_SIZE - start) * selectivity
                    query.append(start)
                    query.append(end)
                queries.append(query)
        elif distribution == Distribution.NORMAL:
            for _ in range(qsize):
                query = []
                for _ in range(dim):
                    start = np.random.uniform(low=DATA_SIZE / 2 - 3 * math.sqrt(DATA_SIZE), high= DATA_SIZE / 2 + 3 * math.sqrt(DATA_SIZE))
                    # normal distribution should use smaller selectivity
                    # otherwise scan range will be huge
                    selectivity = np.random.rand() * 0.01
                    end = start + (DATA_SIZE - start) * selectivity
                    query.append(start)
                    query.append(end)
                queries.append(query)
        # this query distribution may need to be refined
        elif distribution == Distribution.MIX:
            for _ in range(qsize):
                query = []
                model_idx = np.random.randint(low=0, high=dim+1)
                model_start = self.gaussian_model_split[model_idx] * DATA_SIZE
                model_size = self.gaussian_model_split[model_idx+1] * DATA_SIZE
                for _ in range(dim):
                    start = np.random.uniform(low=model_start + model_size / 2 - model_size * 3 / 6,
                    high=model_start + model_size / 2 + model_size * 3 / 6)
                    selectivity = np.random.rand() * 0.01
                    end = start + model_size * selectivity
                    query.append(start)
                    query.append(end)
                queries.append(query)
        return queries

    def _generate_mixGauss_points(self, dim, size):
        # We set the number of Gaussian models to be equal to dim + 1
        num = dim + 3
        model_split = np.random.dirichlet(np.ones(num), 1)
        model_split = np.insert(model_split[0], 0, 0.0)
        self.gaussian_model_split = model_split
        points = None
        for i in range(num):
            model_size = model_split[i+1] * size
            tmp_points = np.random.normal(loc=np.random.uniform(0, DATA_SIZE), 
            scale= model_size/6, size=[int(model_size), dim])
            if i == 0:
                points = tmp_points
            else:
                points = np.concatenate((points, tmp_points), axis=0)
        def _sort_keys(x):
            keys = []
            for i in range(dim):
                keys.append(x[i])
            return keys
        points = np.array(sorted(points, key=_sort_keys))
        # normalize to 0
        points -= np.min(points)
        print(np.min(points))
        return points

    def _generate_random_points(self, dim, size):
        points = np.random.uniform(low=0, high=size, size=[size, dim])
        def _sort_keys(x):
            keys = []
            for i in range(dim):
                keys.append(x[i])
            return keys
        points = np.array(sorted(points, key=_sort_keys))
        return points
    
    def _generate_normal_points(self, dim, size):
        points = np.random.normal(loc=size/2, scale=math.sqrt(size), size=[size, dim])
        def _sort_keys(x):
            keys = []
            for i in range(dim):
                keys.append(x[i])
            return keys
        points = np.array(sorted(points, key=_sort_keys))
        return points

    def _write_to_file(self, data, path):
        with open(path, "w") as f:
            csvWriter = csv.writer(f)
            for idx, row in enumerate(data):
                if idx % 50000 == 0:
                    print("Finish writing points: " + str(idx))
                csvWriter.writerow(row)
        return
    
    def _plot(self, data, path):
        x, y = data.T
        plt.scatter(x, y, s=1)
        plt.savefig(path)
        plt.clf()
        return

g = Generator()
#g.generate(distribution=Distribution.RANDOM)
#g.generate(distribution=Distribution.NORMAL)
g.generate(distribution=Distribution.MIX)