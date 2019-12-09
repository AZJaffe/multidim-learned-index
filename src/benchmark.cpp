#include <array>
#include <cassert>
#include <chrono> 
#include <cstdio>
#include <math.h>
#include <string>
#include <vector>
#include "pi_tree.h"
#include "full_scan.h"

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
results benchmarkPiTree(benchmark<D> & b, uint maxFanout, uint pageSize) {
    results r;
    stringstream ss;
    ss << "PiTree(maxFanout=" << maxFanout << ", pageSize=" << pageSize << ")";
    r.name = ss.str();
    auto start = chrono::steady_clock::now();
    PiTree<D,int> t = PiTree<D, int>(b.data, maxFanout, pageSize);
    r.loadTime = chrono::steady_clock::now() - start;

    start = chrono::steady_clock::now();
    auto it2 = b.max.begin();
    for (auto it = b.min.begin(); it != b.min.end(); it++, it2++) {
        auto v = t.rangeQuery(*it, *it2); // Assign to variable so compiler doesn't optimize this out
        (void)v; // Make sure compiler doesn't optimize out!
    }
    r.queryTime = chrono::steady_clock::now() - start;
    return r;
}

template <uint D>
results benchmarkFullScan(benchmark<D> & b) {
    results r;
    r.name = "Full Scan";
    auto start = chrono::steady_clock::now();
    FullScan<D,int> f = FullScan<D, int>(b.data);
    r.loadTime = chrono::steady_clock::now() - start;
    start = chrono::steady_clock::now();
    auto it2 = b.max.begin();
    for (auto it = b.min.begin(); it != b.min.end(); it++, it2++) {
        auto v = f.rangeQuery(*it, *it2); // Assign to variable so compiler doesn't optimize this out
        (void)v; // Make sure compiler doesn't optimize out!
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
    cout << "  - Type         : " << b.name << endl;
    cout << "  - Dimension    : " << b.min[0].size() << endl;
    cout << "  - # Data points: " << b.data.size() << endl;
    cout << "  - # Queries    : " << b.min.size() << endl;
    cout << endl;
}

int main(void) {
    auto b = uniformRandomDataset<2>(1e7, 1e3, 0.01); // TODO? make the parameters arguments
    printBenchmarkInformation(b);
    auto piTreeResults = benchmarkPiTree(b, 1e3, 5e3);
    printResults(piTreeResults, b);
    auto fullScanResults = benchmarkFullScan(b);
    printResults(fullScanResults, b);
}