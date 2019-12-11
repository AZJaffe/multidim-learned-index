#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <vector>
#include "linear_cdf_regressor.h"
#include "exponential_search.h"
#include "util.h"

#ifdef TRACE
#  define TPRINT(x) cout << x << endl
#  define IFTRACE if(1)
#else
#  define TPRINT(x) do {} while (0)
#  define IFTRACE if(0)
#endif

#ifndef STATS
#define STATS true
#endif

#ifndef FANOUT_FACTOR
#define FANOUT_FACTOR 10
#endif

using namespace std;
using namespace std::chrono;

// D is the dimension of the key of the data, V is the type of the data values
template <uint D, typename V>
class PiTree {
    typedef pair<array<double, D>, V> datum;

    vector<datum> &data; // Data is an array of arrays of doubles of size D
    uint maxFanout; // For now, this is constant
    uint height; // e.g. a tree where the node is a leaf has height 1
    uint pageSize; // This is the upper bound on the page size.

    struct node {
        vector<node *> children;
        array<double, D> proj;
        LinearModel model;
        uint fanout;
        uint start; 
        uint end;
        node() : model(LinearModel(0,0)) {}
        ~node() {
            for(auto n : children) {
                delete n;
            }
        }
        double project(array<double, D> &d) {
            return inner_product(d.begin(), d.end(), proj.begin(), 0.0);
        };
        uint getChildIndex(double d) {
            int prediction = floor(model.predict(d) * fanout);
            return min(fanout - 1, (uint)max(0, prediction));
        };
        uint getIndex(double d) {
            uint prediction = floor(model.predict(d) * (end - start)) + start;
            return max(start, min(prediction, end - 1));
        };
        size_t memorySize() {
            size_t s = sizeof(node);
            s += children.capacity() * sizeof(node *);
            for(uint i = 0; i < children.size(); i++) {
                s += children[i]->memorySize();
            }
            return s;
        }
        uint size() {
            uint ret = 1;
            for(uint i = 0; i < children.size(); i++) {
                ret += children[i]->size();
            }
            return ret;
        }
    };
    node * root;

    struct queryStatistics {
        size_t totalQueries;
        size_t totalHit;
        size_t totalMiss;
        size_t totalLeafSizes;
        size_t totalInternalVisited;
        size_t totalLeavesVisited;
        array<size_t, 20> predictionError; // Index i corresponds to prediction error less than 2^i
        vector<microseconds> queryLatency;
        vector<microseconds> scanLatency;
        vector<microseconds> refineLatency;
        vector<microseconds> traverseLatency;
        queryStatistics() : totalQueries(0), totalHit(0), totalMiss(0), totalLeafSizes(0) {
            for(auto it = predictionError.begin(); it != predictionError.end(); it++) {
                *it = 0;
            }
        }
    };
    queryStatistics stats;

    struct structureData {
        vector<size_t> rangeOfLeaf;
        vector<size_t> depthOfLeaf;
        size_t minDepthOfLeaf;
        size_t maxDepthOfLeaf;
        vector<size_t> fanoutOfInternal;
        size_t numInternal;
        size_t numLeaves;
    };
    

    node * buildSubTree(uint start, uint end, uint depth);
    void collectStructureData(structureData & s, node * n, size_t depth);
    void pairSort(node & n);
    void printSubTree(node * n, uint depth, bool printData = false);
    datum * lookup(array<double, D> query, node * n);
    void rangeQuery(vector<datum> &ret, array<double, D> min, array<double, D> max, node * n, microseconds & refine, microseconds & scan);
    size_t depth(node * n);
    bool isLeaf(node * n) {
        return n->end - n->start < pageSize;
    }
    size_t getPredictionErrorIdx(uint predicted, uint actual) {
        if(predicted - actual == 0) {
            return 0;
        }
        return floor(log2(abs((long)predicted - (long)actual))) + 1;
    }

public:
    PiTree(vector<datum> &data, uint maxFanout, uint pageSize);
    ~PiTree() {
        delete root;
    }
    datum * lookup(array<double, D> query) {
        return lookup(query, root);
    }
    vector<datum> rangeQuery(array<double, D> min, array<double, D> max) {
        for(uint i = 0; i < D; i++) assert(min[i] <= max[i]);
        assert(root != nullptr);
        steady_clock::time_point start; if(STATS) start = steady_clock::now();
        microseconds refineTime; microseconds scanTime; 
        if(STATS) {
            refineTime = microseconds(0);
            scanTime = microseconds(0);
        }
        vector<datum> ret;
        rangeQuery(ret, min, max, root, refineTime, scanTime);
        steady_clock::time_point end; if(STATS) end = steady_clock::now();
        if (STATS) {
            auto qTime = duration_cast<microseconds>(end - start);
            stats.queryLatency.push_back(qTime);
            stats.scanLatency.push_back(scanTime);
            stats.refineLatency.push_back(refineTime);
            stats.traverseLatency.push_back(qTime - scanTime - refineTime);
            stats.totalQueries++;
            stats.totalHit += ret.size();
        }
        return ret;
    }
    void printTree(bool printData = false) {
        printSubTree(root, 0, printData);
    }
    void printTreeStats();
    void printQueryStats();
    void resetQueryStats() {
        stats = queryStatistics();
    }
    size_t depth() {
        return depth(root);
    }
    size_t memorySize() {
        return sizeof(PiTree<D,V>) + root->memorySize() - sizeof(queryStatistics);
    }
    uint size() {
        return root->size();
    }
};

template <uint D, typename V>
PiTree<D,V>::PiTree(vector<datum> &data, uint maxFanout, uint pageSize) :
data(data), maxFanout(maxFanout), pageSize(pageSize) {
    assert(maxFanout > 0);
    assert(pageSize >= 1);
    root = buildSubTree(0, data.size(), 0);
}

template <uint D, typename V>
typename PiTree<D,V>::node * PiTree<D,V>::buildSubTree(uint start, uint end, uint depth) {
    TPRINT("Building subtree with start=" << start << " end=" << end);
    node * n = new node();
    n->start = start;
    n->end = end;
    for(uint i = 0; i < D; i++) {
        n->proj[i] = (i == depth % D) ? 1 : 0;
    }
    if(end - start == 0) {
        return n;
    }
    pairSort(*n);
    LinearCdfRegressor builder = LinearCdfRegressor();
    for (uint i = start; i < end; i++) {
        builder.add(
            n->project(data[i].first)
        );
    }
    n->model = builder.fit();
    TPRINT("regressor=(" << n->model.slope << "x + " << n->model.bias << ")");
    if (isLeaf(n)) {
        n->fanout = 0;
        return n;
    }
    n->fanout = min(maxFanout, FANOUT_FACTOR * (uint)ceil((double)(end - start) / pageSize));
    uint childStart = start;
    double childMaxVal = 1.0 / n->fanout;
    double maxValIncrement = childMaxVal;
    for(uint i = start; i < end; i++) {
        double p = n->model.predict(
            n->project(data[i].first)
        );
        while (p >= childMaxVal) {
            n->children.push_back(
                buildSubTree(childStart, i, depth+1)
            );
            childStart = i;
            if (n->children.size() == n->fanout - 1) {
                childMaxVal = numeric_limits<double>::max();
            } else {
                childMaxVal += maxValIncrement;
            }
        }
    }
    // n->children.size() might not equal the fanout
    // e.g. if the fanout is 2 and the predictions for all datapoints lie within [0, 0.2], there will only be one child.
    n->children.push_back(
        buildSubTree(childStart, end, depth+1)
    );
    return n;
}

template<uint D, typename V>
void PiTree<D,V>::rangeQuery(vector<typename PiTree<D,V>::datum> &ret, array<double, D> min, array<double, D> max, node * n, microseconds & refine, microseconds & scan) {
    double minProjection = 0; double maxProjection = 0;
    for(uint i = 0; i < D; i++) {
        double minProduct = n->proj[i] * min[i];
        double maxProduct = n->proj[i] * max[i];
        if(n->proj[i] > 0) {
            minProjection += minProduct;
            maxProjection += maxProduct;
        } else {
            minProjection += maxProduct;
            maxProjection += minProduct; 
        }
    }

    if(isLeaf(n)) {
        steady_clock::time_point before, after;
        if (STATS) {
            stats.totalLeavesVisited++;
            stats.totalLeafSizes += n->end - n->start;
            before = steady_clock::now(); // Start of refinement
        }

        uint predictedStart = n->getIndex(minProjection);
        uint predictedEnd = n->getIndex(maxProjection);
        // The predicted indices could be off. Have to do exponential search to find the actual minIndex,maxIndex to search

        // 2 ways of doing this
        // first is the get lower bound then iterate and check every stage
        // second is to get lower and upper bound, then iterate between (without checking)
        // Doing it the second way, maybe first is faster?
        auto start = exponentialSearchLowerBound(data.begin() + n->start, data.begin() + n->end, data.begin() + predictedStart, minProjection,
            [n](datum &d, double p) { return n->project(d.first) < p; });
        auto end = exponentialSearchUpperBound(data.begin() + n->start, data.begin() + n->end, data.begin() + predictedEnd, maxProjection,
            [n](double p, datum &d) { return p < n->project(d.first); });


        size_t beforeSize;
        if (STATS) {
            after = steady_clock::now(); // End of refinement and start of scanning
            refine += duration_cast<microseconds>(after - before);
            stats.predictionError[getPredictionErrorIdx(predictedStart, start - data.begin())]++;
            stats.predictionError[getPredictionErrorIdx(predictedEnd, end - data.begin())]++;
            beforeSize = ret.size();
            before = after;
        }

        TPRINT("Range scanning node with range [" << n->start << ", " << n->end << ") on subrange [" << start - data.begin() << ", " << end - data.begin() << ")");
        for(auto it = start; it < end; it++) {
            if (withinRange<D>(it->first, min, max)) {
                ret.push_back(*it);
            }
        }
        if (STATS) {
            after = steady_clock::now();
            scan += duration_cast<microseconds>(after - before);
            // # missed = (# scanned) - (# hit)
            stats.totalMiss += (end - start) - (ret.size() - beforeSize);
        }
        return;
    } else {
        if (STATS) stats.totalInternalVisited++;
        uint minChildIndex = n->getChildIndex(minProjection);
        uint maxChildIndex = n->getChildIndex(maxProjection);

        for(uint i = minChildIndex; i <= maxChildIndex && i < n->children.size(); i++) {
            rangeQuery(ret, min, max, n->children[i], refine, scan);
        }
        return;
    }
}

template <uint D, typename V>
typename PiTree<D,V>::datum * PiTree<D,V>::lookup(array<double, D> query, node * n) {
    if(isLeaf(n)) {
        double projQuery = n->project(query);
        uint prediction = n->getIndex(projQuery);
        uint p = max(n->start, min(prediction, n->end));
        auto it = exponentialSearchLowerBound(data.begin() + n->start, data.begin() + n->end, data.begin() + p, projQuery, 
            [n](datum &d, double p) { return n->project(d.first) < p; });
        while(it < data.begin() + n->end && projQuery >= n->project(it->first)) {
            bool equal = true;
            for(uint i = 0; i < D; i++) {
                if (compare(query[i], it->first[i]) != 0) {
                    equal = false;
                    break;
                }
            }
            if (equal) {
                return &(*it);
            }
            it++;
        }
        return nullptr;
    }
    double projQuery = n->project(query);
    uint childIndex = n->getChildIndex(projQuery);
    if(childIndex < n->children.size()) {
        return lookup(query, n->children[childIndex]);
    } else {
        return nullptr;
    }
}

// sorts the vector of data between indices start and end according
// to the ordering induced by the linear functional proj
template <uint D, typename V>
void PiTree<D,V>::pairSort(node & n) {
    int length = n.end - n.start;
    vector<pair<double, datum>> paired; // TODO do this without copying
    for(int i = 0; i < length; i++) {
        paired.push_back(make_pair(
            n.project(data[i+n.start].first),
            data[i+n.start]
        ));
    }
    sort(paired.begin(), paired.end());
    for(int i = 0; i < length; i++) {
        data[i+n.start] = paired[i].second;
    }
    return;
}

template <uint D, typename V>
void PiTree<D,V>::printSubTree(node * n, uint depth, bool printData) {
    cout << string((int)depth * 2, ' ') << "- ";
    if(n->start == n->end) {
        cout << "NULL" << endl;
        return;
    }
    cout << "range=[" << n->start << ", " << n->end << ") ";
    cout << "proj=[";
    for(uint i = 0; i < D-1; i++) {
        cout << n->proj[i] << ",";
    }
    cout << n->proj[D-1] << "] ";
    cout << "regressor=(" << n->model.slope << "x + " << n->model.bias << ") ";
    cout << "fanout=" << n->fanout;
    cout << endl;
    if(isLeaf(n) && printData) {
        for(uint i = n->start; i < n->end; i++) {
            cout << string((int)depth * 2 + 2, ' ') << "- {[";
            for(uint d = 0; d < D-1; d++) {
                cout << data[i].first[d] << ", ";
            }
            cout << data[i].first[D-1] << "], ";
            cout << data[i].second << "}" << endl;
        }
    }
    if(!isLeaf(n)) {
        uint i = 0;
        for(; i < n->children.size(); i++) {
            printSubTree(n->children[i], depth+1, printData);
        }
        // The number of children might be less than the fanout
        // For the rest, just print empty ranges.
        for(; i < n->fanout; i++) {
            cout << string((1 + (int)depth) * 2, ' ') << "- ";
            cout << "range=[" << n->end << ", " << n->end << ")" << endl;
        }
    }
}

template <uint D, typename V>
size_t PiTree<D,V>::depth(node * n) {
    if(isLeaf(n)) {
        return 1;
    }
    size_t max = 1;
    for(auto it = n->children.begin(); it != n->children.end(); it++) {
        size_t childDepth = depth(*it);
        max = childDepth > max ? childDepth : max;
    }
    return max;
}

template <uint D, typename V>
void PiTree<D,V>::printTreeStats() {
    structureData s;
    s.rangeOfLeaf = vector<size_t>();
    s.depthOfLeaf = vector<size_t>();
    s.fanoutOfInternal = vector<size_t>();
    s.minDepthOfLeaf = numeric_limits<size_t>::max();
    s.maxDepthOfLeaf = 0;
    s.numInternal = 0;
    s.numLeaves = 0;
    collectStructureData(s, root, 1);

    // depth[i] is # of leaves at depth i
    size_t * depth = new size_t[s.maxDepthOfLeaf + 1]();
    // historgrams with 10 buckets
    const size_t nFanoutBuckets = 100;
    const size_t nRangeBuckets = 10;
    size_t * fanoutBuckets = new size_t[nFanoutBuckets]();
    size_t * rangeBuckets = new size_t[nRangeBuckets]();

    for(auto it = s.depthOfLeaf.begin(); it != s.depthOfLeaf.end(); it++) {
        depth[*it]++;
    }

    for(auto it = s.rangeOfLeaf.begin(); it != s.rangeOfLeaf.end(); it++) {
        size_t idx = floor((double)*it / (double)pageSize * nRangeBuckets);
        rangeBuckets[idx]++;
    }

    for(auto it = s.fanoutOfInternal.begin(); it != s.fanoutOfInternal.end(); it++) {
        size_t idx = floor((double)*it / (double)maxFanout * nFanoutBuckets);
        if (idx == nFanoutBuckets) {
            idx = nFanoutBuckets - 1;
        }
        fanoutBuckets[idx]++;
    }

    cout << "PiTree Statistics:" << endl;
    cout << "  - Max Fanout   : " << maxFanout << endl;
    cout << "  - Page Size    : " << pageSize << endl;
    cout << "  - Num Nodes    :  " << s.numInternal + s.numLeaves << endl;
    cout << "  - Num Leaves   : " << s.numLeaves << endl;
    cout << "  - Num Internal : " << s.numInternal << endl;
    cout << "  - Depth Stats  :" << endl;
    for(size_t i = s.minDepthOfLeaf; i <= s.maxDepthOfLeaf; i++) {
        cout << "    * Leaves at depth " << i << ": " << depth[i] << endl;
    }
    cout << "  - Fanout Stats:" << endl;
    for(size_t i = 0; i < nFanoutBuckets; i++) {
        cout << "    * [" << i * maxFanout / nFanoutBuckets << ", " << (i+1) * maxFanout / nFanoutBuckets << "): " << fanoutBuckets[i] << endl;
    }

    cout << "  - Leaf Range Stats:" << endl;
    for(size_t i = 0; i < nRangeBuckets; i++) {
        cout << "    * [" << i * pageSize / nRangeBuckets << ", " << (i+1) * pageSize / nRangeBuckets << "): " << rangeBuckets[i] << endl;
    }

    delete[] depth;
    delete[] fanoutBuckets;
    delete[] rangeBuckets;
}

template <uint D, typename V>
void PiTree<D,V>::collectStructureData(PiTree<D,V>::structureData & s, PiTree<D,V>::node * n, size_t depth) {
    if(isLeaf(n)) {
        s.numLeaves++;
        size_t range = n->end - n->start;
        s.minDepthOfLeaf = depth < s.minDepthOfLeaf ? depth : s.minDepthOfLeaf;
        s.maxDepthOfLeaf = depth > s.maxDepthOfLeaf ? depth : s.maxDepthOfLeaf;
        s.depthOfLeaf.push_back(depth);
        s.rangeOfLeaf.push_back(range);
    } else {
        s.numInternal++;
        s.fanoutOfInternal.push_back(n->fanout);
        for(auto it = n->children.begin(); it != n->children.end(); it++) {
            collectStructureData(s, *it, depth + 1);
        }
    }
}


template <uint D, typename V>
void PiTree<D,V>::printQueryStats() {

    if (!STATS) return;
    if (stats.totalQueries < 1) return;

    sort(stats.queryLatency.begin(), stats.queryLatency.end()); // sort to get the percentiles
    auto avgLatency = accumulate(stats.queryLatency.begin(), stats.queryLatency.end(), microseconds(0)).count() / stats.totalQueries;
    auto avgScanLatency = accumulate(stats.scanLatency.begin(), stats.scanLatency.end(), microseconds(0)).count() / stats.totalQueries;
    auto avgRefineLatency = accumulate(stats.refineLatency.begin(), stats.refineLatency.end(), microseconds(0)).count() / stats.totalQueries;
    auto avgTraverseLatency = accumulate(stats.traverseLatency.begin(), stats.traverseLatency.end(), microseconds(0)).count() / stats.totalQueries;

    cout << "PiTree Range Query Statistics:" << endl;
    cout << "  - Queries                    : " << stats.totalQueries << endl;
    cout << "  - Avg Scanned                : " << (double)(stats.totalHit + stats.totalMiss) * 100 / data.size() / stats.totalQueries << "%" << endl;
    cout << "  - Avg Hit/Scanned            : " << (double)stats.totalHit * 100 / (stats.totalHit + stats.totalMiss) << "%" << endl;
    cout << "  - Avg Leaves Searched/Query  : " << (double)stats.totalLeavesVisited / stats.totalQueries << endl;
    cout << "  - Avg Internal Visited/Query : " << (double)stats.totalInternalVisited / stats.totalQueries << endl;
    cout << "  - Avg Refinement             : " << (double)(stats.totalHit + stats.totalMiss) * 100  / stats.totalLeafSizes << "%" << endl;
    cout << "  - Median Latency             : " << stats.queryLatency[stats.queryLatency.size() * 50 / 100].count() << "μs" << endl;
    cout << "  - 95th Percentile Latency    : " << stats.queryLatency[stats.queryLatency.size() * 95 / 100].count() << "μs" << endl;
    cout << "  - 99th Percentile Latency    : " << stats.queryLatency[stats.queryLatency.size() * 99 / 100].count() << "μs" << endl;
    cout << "  - 99.9th Percentile Latency  : " << stats.queryLatency[stats.queryLatency.size() * 999 / 1000].count() << "μs" << endl;
    cout << "  - Max Latency                : " << stats.queryLatency[stats.queryLatency.size() - 1].count() << "μs" << endl;
    cout << "  - Avg Latency                : " << avgLatency << "μs" << endl;
    cout << "  - Avg Scan Latency           : " << avgScanLatency << "μs" << endl;
    cout << "  - Avg Refine Latency         : " << avgRefineLatency << "μs" << endl;
    cout << "  - Avg Traverse Latency       : " << avgTraverseLatency << "μs" << endl;
    cout << "  - Prediction Error Stats     : " << endl;
    // 2x since each leaf visited has two predictions - start and end.
    cout << "    * [0, 0]: " << (double)stats.predictionError[0] / (2 * stats.totalLeavesVisited) * 100 << "%" << endl;
    size_t total = 2 * stats.totalLeavesVisited - stats.predictionError[0];
    for (size_t i = 1; i < stats.predictionError.size(); i++) {
        if (total < 1) break;
        total -= stats.predictionError[i];
        int start = pow(i - 1, 2);
        int end = pow(i, 2);
        cout << "    * [" << start << ", " << end << "): " << (double)stats.predictionError[i] / (2 * stats.totalLeavesVisited) * 100 << "%" << endl;
    }
    cout << endl;
}