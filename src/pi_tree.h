#pragma once

// 10 is arbitrary
#define MAX_FANOUT 10

#ifdef DEBUG_BUILD
#  define DEBUG(x) cout << x
#else
#  define DEBUG(x) do {} while (0)
#endif

using namespace std;
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <vector>
#include "linear_cdf_regressor.h"

// D is the dimension of the key of the data, V is the type of the data values
template <uint D, typename V>
class PiTree {
    typedef pair<array<double, D>, V> datum;

    vector<datum> &data; // Data is an array of arrays of doubles of size D
    uint fanout; // For now, this is constant
    uint height; // e.g. a tree where the node is a leaf has height 1
    uint pageSize; // This is the upper bound on the page size.

    struct node {
        vector<node *> children;
        array<double, D> proj;
        LinearModel model;
        bool isLeaf;
        int start; 
        int end;
        node() : model(LinearModel(0,0)) {}
        double project(datum &d) {
            return inner_product(d.first.begin(), d.first.end(), proj.begin(), 0.0);
        };
    };
    node * root;

    node * buildSubTree(uint start, uint end, uint depth);
    void pairSort(node & n);
    void printSubTree(node * n, uint depth);
    datum * lookup(array<double, D> query, node * n);
    datum * searchLeaf(array<double, D> query, node * n);

public:
    PiTree(vector<datum> &data, uint fanout, uint pageSize);
    datum * lookup(array<double, D> query) {
        return lookup(query, root);
    }
    void printTree() {
        printSubTree(root, 0);
    }
};

template <uint D, typename V>
PiTree<D,V>::PiTree(vector<datum> &data, uint fanout, uint pageSize) :
data(data), fanout(fanout), pageSize(pageSize) {
    assert(fanout > 1);
    assert(pageSize > 1);
    root = buildSubTree(0, data.size(), 0);
}

template <uint D, typename V>
typename PiTree<D,V>::node * PiTree<D,V>::buildSubTree(uint start, uint end, uint depth) {
    DEBUG("Building subtree with start=" << start << " end=" << end << endl);
    node * n = new node();
    n->start = start;
    n->end = end;
    for(uint i = 0; i < D; i++) {
        n->proj[i] = (i == depth % D) ? 1 : 0;
    }
    pairSort(*n);
    LinearCdfRegressor builder = LinearCdfRegressor();
    for (size_t i = 0; i < data.size(); i++) {
        builder.add(
            n->project(data[i])
        );
    }
    n->model = builder.fit();
    DEBUG("regressor=(" << n->model.slope << "x + " << n->model.bias << ")" << endl);
    n->isLeaf = (end - start < pageSize);
    if (!n->isLeaf) {
        uint childStart = 0;
        double childMaxVal = 1.0 / (double)fanout;
        double maxValIncrement = childMaxVal;
        DEBUG("childMaxVal=" << childMaxVal << endl);
        for(uint i = start; i < end; i++) {
            double p = n->model.predict(
                n->project(data[i])
            );
            DEBUG("p=" << p << " proj=" << n->project(data[i]) << endl);
            while (p >= childMaxVal) {
                n->children.push_back(
                    buildSubTree(childStart, i, depth+1)
                );
                childStart = i;
                if (n->children.size() == fanout - 1) {
                    childMaxVal = numeric_limits<double>::max();
                } else {
                    childMaxVal += maxValIncrement;
                }
                DEBUG("childMaxVal=" << childMaxVal << endl);
            }
        }
        while(n->children.size() < fanout) {
            n->children.push_back(
                buildSubTree(childStart, end, depth+1)
            );
            childStart = end;
        }
        assert(n->children.size() == fanout);
    }
    return n;
}

template <uint D, typename V>
typename PiTree<D,V>::datum * PiTree<D,V>::lookup(array<double, D> query, node * n) {
    if(n->isLeaf) {
        return searchLeaf(query, n);   
    }
    double projQuery = n->project(query);
    double prediction = n->model.predict(projQuery);
    uint childIndex = clamp(
        floor(prediction * n->children.size()),
        0, n->chidren.size() -1
    );
    return lookup(query, n->children[childIndex]);
}


// Leaf search is a little complicated.
// Part of the complexity arises from the fact that even if n->project(data[i]) == projQuery
// it doesn't mean that query == data[i].first. 
// Once leaf search finds such an i, call localSearch which will use equality across all dimensions to verify a match in a segment around i.

// Leaf search is a 4 step process:
// 1. Find leftBound in [start, end] such that n->project(data[leftBound]) < projQuery, using exponential search
// 2. Find rightBound in [start, end] such that n->project(data[rightBound]) > projQuery, using exponential search
// 3. Do binary search to find an index i such that n->project(data[i]) == projQuery
// 4. Do local search around i to see if any values in the dataset are strictly equal to query.

// Either step 1 or 2 will run, not both.
// If an index i such that n->project(data[i]) == projQuery is found, then immediately do local search
template <uint D, typename V>
typename PiTree<D,V>::datum * PiTree<D,V>::searchLeaf(array<double, D> query, node * n) {
    assert(n->isLeaf);
    double projQuery = n->project(query);
    double prediction = n->model.predict(projQuery);
    uint p = clamp(
        floor(prediction * (n->end - n->start)),
        n->start, n->end
    );
    int c = compare(n->project(data[p]), projQuery);
    int rightBound = p;
    int leftBound = p;
    if (c == 0) {
        return localSearch(p, n->start, n->end);
    } else if (c > 0) {
        // In this case, n->project(data[p]) > projQuery, and so p is a rightBound
        // Now we have to find leftBound.
        // To do that, exponential search to the left.
        int gap = 1;
        while (c < 0 && leftBound >= n->start) {
            leftBound = max(leftBound - gap, n->start);
            gap *= 2;
            c = compare(n->project(data[leftBound]), projQuery);
        }
        if (c == 0) {
            return localSearch(leftBound, n->start, n->end);
        } else if (c < 0) { // => leftBound = n->start
            return nullptr;
        }
    } else {
        // In this case, n->project(data[p]) < projQuery, and so p is a leftBound
        // Now we have to find a rightBound.
        // To do that, exponential search to the right.
        int gap = 1;
        while(c > 0 && rightBound < n->end) {
            rightBound = min(rightBound + gap, n->end - 1);
            gap *= 2;
            c = compare(n->project(data[rightBound]), projQuery);
        }
        if(c == 0) {
            return localSearch(rightBound, n->start, n->end);
        } else if (c > 0) { // => rightBound = n->end - 1
            return nullptr;
        }
    }
    // Time for binary search in the open interval (leftBound, rightBound)
    uint step = (rightBound - leftBound) / 2; // ???? maybe off by 1
    while(step > 0) {
        // TODO
    }
}

// sorts the vector of data between indices start and end according
// to the ordering induced by the linear functional proj
template <uint D, typename V>
void PiTree<D,V>::pairSort(node & n) {
    int length = n.end - n.start;
    vector<pair<int, datum>> paired; // TODO do this without copying
    for(int i = 0; i < length; i++) {
        paired.push_back(make_pair(
            n.project(data[i+n.start]),
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
void PiTree<D,V>::printSubTree(node * n, uint depth) {
    cout << string((int)depth * 2, ' ') << "- ";
    cout << "start=" << n->start << " end=" << n->end;
    cout << " proj=[";
    for(uint i = 0; i < D-1; i++) {
        cout << n->proj[i] << ",";
    }
    cout << n->proj[D-1] << "] ";
    cout << "regressor=(" << n->model.slope << "x + " << n->model.bias << ")";
    cout << endl;
    for(uint i = 0; i < n->children.size(); i++) {
        printSubTree(n->children[i], depth+1);
    }
}

int compare(double a, double b) {
    if (abs(a - b) < numeric_limits<double>::epsilon()) {
        return 0;
    }
    return a < b ? -1 : 1;
}

uint clamp(uint v, uint lb, uint ub) {
    return max(lb, min(ub, v));
}

