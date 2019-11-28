import argparse, csv, math, random, string
import numpy as np
import os, sys
import matplotlib.pyplot as plt

import config

dataSize = config.DATA_SIZE
pageSize = config.PAGE_SIZE

def generate_data_distribution(distribution):
    if distribution == config.Distribution.SEQUENCE:
        data = np.arange(dataSize)
    elif distribution == config.Distribution.RANDOM:
        data = np.random.randint(low=0, high=dataSize * 5, size=dataSize)
    elif distribution == config.Distribution.EXPONENTIAL:
        data = np.random.exponential(5, size=dataSize)
        data *= 10000
    else:
        raise RuntimeError("Unsupported Distribution")
    
    data = data.astype(int)
    return np.sort(data)

def validate_data(data):
    current = data[0]
    count = 0
    for i in range(1, len(data)):
        if current == data[i]:
            for k in range(i, len(data)):
                data[k] += 1
            count += 1
        current = data[i]
    print("Num of duplicated keys: " + str(count))
    current = data[0]
    for i in range(1, len(data)):
        if current == data[i]:
            print("Duplicated key. Should never happen!")
            assert(1==0)
        current = data[i]

def plot_data(data, distribution):
    plt.plot(data)
    scale = data[-1]
    normData = data * scale / data.sum()
    cdata = np.cumsum(normData)
    plt.plot(cdata, 'r--')
    plt.savefig(config.dataDistributionFile[distribution])
    plt.clf()

def write_data(data, distribution):
    with open(config.sparseIdxFile[distribution], "w") as f:
        csvWriter = csv.writer(f)
        csvWriter.writerow(["Index", "Key"])
        for index, key in enumerate(data):
            csvWriter.writerow([int(index / pageSize), key])
    
    with open(config.denseIdxFile[distribution], "w") as f:
        csvWriter = csv.writer(f)
        csvWriter.writerow(["Index", "Key"])
        for index, key in enumerate(data):
            csvWriter.writerow([int(index), key])

def generate_data(distribution):
    data = generate_data_distribution(distribution)
    validate_data(data)
    plot_data(data, distribution)
    write_data(data, distribution)

if __name__ == "__main__":
    for distribution in config.Distribution:
        print("Generate data for: " + str(distribution))
        generate_data(distribution)