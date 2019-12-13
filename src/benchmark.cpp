#include <array>
#include <cassert>
#include <chrono> 
#include <cstdio>
#include <iostream>
#include <math.h>
#include <string>
#include <sstream>
#include <vector>
#include "kd_tree.h"
#include "pi_tree.h"
#include "full_scan.h"
#include "r_tree.h"
#include "util.h"

using namespace std;

template <uint D>
struct benchmark {
    vector<pair<array<double, D>, int>> data;
    vector<array<double, D>> min; // min and max are used to represent range queries. They should have the same length.
    vector<array<double, D>> max;
    double selectivity;
    string name;
};

struct results {
    chrono::duration<double> loadTime;
    chrono::duration<double> queryTime;
    string name;
    vector<size_t> resultSetSize;
};

template <uint D>
benchmark<D> uniformRandomDataset(size_t numData, size_t numQueries, double selectivity) { // selectivity is the percent of selectivity for each query (approximately)
    auto data = vector<pair<array<double, D>, int>>(numData);
    benchmark<D> b = {
        vector<pair<array<double, D>, int>>(numData),
        vector<array<double, D>>(numQueries),
        vector<array<double, D>>(numQueries),
        selectivity,
        "Uniform random dataset and queries"
    };
    for(auto itd = b.data.begin(); itd != b.data.end(); itd++) {
        array<double, D> k;
        for(auto it = k.begin(); it != k.end(); it++) {
            *it = static_cast<double>(rand()) / static_cast<double>(RAND_MAX) * 1.0;
        }
        int v = itd - b.data.begin();
        *itd = make_pair(k, v);
    }
    // Assuming independence in dimensions, s = p^d, where s is selectivity, p is the per dimension selectivity, 
    // and d is the number of dimensions
    double perDimensionSelectivity = pow(selectivity, 1.0 / (double)D);
    auto it2 = b.max.begin();
    for(auto it = b.min.begin(); it != b.min.end(); it++, it2++) {
        for(auto [minIt, maxIt] = make_pair(it->begin(), it2->begin()); minIt != it->end(); minIt++, maxIt++) {
            *minIt = static_cast<double>(rand()) / static_cast<double>(RAND_MAX) * 1.0;
            *maxIt = *minIt + perDimensionSelectivity;
        }
    }
    return b;
}

template <uint D>
benchmark<D> loadData(size_t numData, size_t numQueries, string distribution) {
    auto data = vector<pair<array<double, D>, int>>(numData);
    benchmark<D> b = {
        vector<pair<array<double, D>, int>>(numData),
        vector<array<double, D>>(numQueries),
        vector<array<double, D>>(numQueries),
        -1, // loaded data use random selectivity, this is a dummy entry
        to_string(D) + "D-" + distribution
    };
    string dataFile = "../data/" + b.name + "/points.csv";
    CSVReader readerD(dataFile);
    vector<vector<string>> dataList = readerD.getData();
    for (uint i = 0; i < numData; ++i) {
        array<double, D> k;
        for (uint j = 0; j < D; ++j) {
            k[j] = stod(dataList[i][j]);
        }
        auto p = make_pair(k, i);
        b.data[i] = p;
    }
    string queryFile = "../data/" + b.name +  "/queries.csv";
    CSVReader readerQ(queryFile);
    vector<vector<string>> queryList = readerQ.getData();
    for (uint i = 0; i < numQueries; ++i) {
        array<double, D> minQ;
        array<double, D> maxQ;
        for (uint j = 0; j < D; ++j) {
            minQ[j] = stod(queryList[i][2 * j]);
            maxQ[j] = stod(queryList[i][2 * j + 1]);
        }
        b.min[i] = minQ;
        b.max[i] = maxQ;
    }
    return b;
}

template <uint D>
results benchmarkPiTree(benchmark<D> & b, uint maxFanout, uint pageSize) {
    results r;
    stringstream ss;
    ss << "PiTree(maxFanout=" << maxFanout << ", pageSize=" << pageSize << ")";
    r.name = ss.str();
    auto start = chrono::steady_clock::now();
    PiTree<D,int> t = PiTree<D, int>(b.data, maxFanout, pageSize);
    r.loadTime = chrono::steady_clock::now() - start;

    t.printTreeStats();

    start = chrono::steady_clock::now();
    auto it2 = b.max.begin();
    for (auto it = b.min.begin(); it != b.min.end(); it++, it2++) {
        r.resultSetSize.push_back(t.rangeQuery(*it, *it2).size());
    }
    r.queryTime = chrono::steady_clock::now() - start;
    t.printQueryStats();
    return r;
}

template <uint D>
results benchmarkFullScan(benchmark<D> & b) {
    results r;
    r.name = "FullScan";
    auto start = chrono::steady_clock::now();
    FullScan<D,int> f = FullScan<D, int>(b.data);
    r.loadTime = chrono::steady_clock::now() - start;
    start = chrono::steady_clock::now();
    auto it2 = b.max.begin();
    for (auto it = b.min.begin(); it != b.min.end(); it++, it2++) {
        r.resultSetSize.push_back(f.rangeQuery(*it, *it2).size());
    }
    r.queryTime = chrono::steady_clock::now() - start;
    return r;
}

template <uint D>
results benchmarkKDTree(benchmark<D> & b) {
    results r;
    r.name = "KdTree";
    auto start = chrono::steady_clock::now();
    KdTree<D, int> t = KdTree<D, int>(b.data);
    r.loadTime = chrono::steady_clock::now() - start;
    auto it2 = b.max.begin();
    start = chrono::steady_clock::now();
    for (auto it = b.min.begin(); it != b.min.end(); it++, it2++) {
        r.resultSetSize.push_back(t.rangeQuery(*it, *it2).size());
    }
    r.queryTime = chrono::steady_clock::now() - start;
    return r;
}

template <uint D>
results benchmarkRTree(benchmark<D> & b) {
    results r;
    r.name = "R*-Tree";
    auto start = chrono::steady_clock::now();
    // CustomRTree<D,int> rt = CustomRTree<D, int>(b.data);
    CustomRTree<D,int> rt = CustomRTree<D,int>("../data/2D-random/points.csv");
    cout << "RTree finished building" << endl;
    r.loadTime = chrono::steady_clock::now() - start;
    start = chrono::steady_clock::now();
    auto it2 = b.max.begin();
    for (auto it = b.min.begin(); it != b.min.end(); it++, it2++) {
        r.resultSetSize.push_back(rt.rangeQuery(*it, *it2));
    }
    r.queryTime = chrono::steady_clock::now() - start;
    return r;
}

template<uint D>
void printResults(results &r, benchmark<D> & b) {
    int64_t loadTime = chrono::duration_cast<chrono::milliseconds>(r.loadTime).count();
    int64_t queryTime = chrono::duration_cast<chrono::milliseconds>(r.queryTime).count();
    cout << "Results for " << r.name << ":" << endl;
    cout << "  - Load Time: " << loadTime << "ms" << endl;
    cout << "  - Query Time: " << queryTime << "ms" << endl;
    cout << "  - Total Time: " << loadTime + queryTime << "ms" << endl;
    cout << "  - Time/Query: " << (double)queryTime / b.max.size() << "ms" << endl;
    cout << endl;
}

template<uint D>
void printBenchmarkInformation(benchmark<D> &b) {
    cout << "Benchmark Details:" << endl;
    cout << "  - Type           : " << b.name << endl;
    cout << "  - Dimension      : " << b.min[0].size() << endl;
    cout << "  - # Data points  : " << b.data.size() << endl;
    cout << "  - # Queries      : " << b.min.size() << endl;
    cout << "  - Avg Selectivity: " << b.selectivity << endl;
    cout << endl;
}

void evaluate(string distribution, uint numData, uint numQueries, uint maxFanout, uint pageSize) {
    cout << "==============================================" << endl;
    auto b = loadData<2>(numData, numQueries, distribution);
    // auto b = uniformRandomDataset<2>(numData, numQueries, 0.1);
    printBenchmarkInformation(b);
    auto fullScanResults = benchmarkFullScan(b);
    auto kdTreeResults = benchmarkKDTree(b);
    auto piTreeResults = benchmarkPiTree(b, maxFanout, pageSize);
    // auto RTreeResults = benchmarkRTree(b);
    printResults(fullScanResults, b);
    printResults(kdTreeResults, b);
    printResults(piTreeResults, b);
<<<<<<< HEAD
    printResults(RTreeResults, b); 
    printResults(fullScanResults, b);
=======
    // printResults(RTreeResults, b); 
>>>>>>> 830707c29e49d2a6c1f1372be30b887234904a33
    for(size_t i = 0; i < fullScanResults.resultSetSize.size(); i++) {
        assert(fullScanResults.resultSetSize[i] == piTreeResults.resultSetSize[i]);
        assert(fullScanResults.resultSetSize[i] == kdTreeResults.resultSetSize[i]);
        // assert(fullScanResults.resultSetSize[i] == RTreeResults.resultSetSize[i]);
    }
}

int main(void) {
    evaluate("random", 1e6, 1e3, 1e3, 5e3);
    // TODO parse parameters to decide what benchmarks to run and what indices to use
<<<<<<< HEAD
    evaluate("random", 1e6, 1e3, 1e3, 5e2);
=======
>>>>>>> 830707c29e49d2a6c1f1372be30b887234904a33
    // evaluate("normal", 1e6, 1e3, 1e4, 5e3);
    // evaluate("mix-Gauss", 1e6, 1e3, 1e3, 5e2);
}