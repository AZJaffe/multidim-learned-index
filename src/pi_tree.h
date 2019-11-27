#pragma once

// 10 is arbitrary
#define MAX_FANOUT 10

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

template <uint D>
double dotProduct(const array<double, D> &v, const array<double, D> &w) {
    return inner_product(v.begin(), v.end(), w.begin(), 0);
}

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
        double project(datum d) {
            return dotProduct<D>(d.first, proj);
        }
    };
    node * root;

    node * buildSubTree(uint start, uint end, uint depth);
    void pairSort(node * n);
    void printSubTree(node * n, uint depth);
    datum * lookup(array<double, D> query, node * n);

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
    node * n = new node();
    n->start = start;
    n->end = end;
    for(uint i = 0; i < D; i++) {
        n->proj[i] = (i == depth % D) ? 1 : 0;
    }
    pairSort(n);
    LinearCdfRegressor builder = LinearCdfRegressor();
    for (size_t i = 0; i < data.size(); i++) {
        builder.add(
            n->project(data[i])
        );
    }
    n->model = builder.fit();
    n->isLeaf = (end - start < pageSize);
    if (!n->isLeaf) {
        uint childStart = 0;
        double childMaxVal = 0;
        double maxValIncrement = 1 / (double)fanout;
        for(uint i = start; i < end; i++) {
            double p = n->model.predict(
                n->project(data[i])
            );
            while (p >= childMaxVal) {
                n->children.push_back(
                    buildSubTree(childStart, i, depth+1)
                );
                childStart = i;
                if (n->children.size() == fanout) {
                    childMaxVal = numeric_limits<double>::max();
                } else {
                    childMaxVal += maxValIncrement;
                }
            }
        }
        while(n->children.size() < fanout) {
            n->children.push_back(
                buildSubTree(end, end, depth+1)
            );
        }
        assert(n->children.size() == fanout);
    }
    return n;
}

template <uint D, typename V>
typename PiTree<D,V>::datum * PiTree<D,V>::lookup(array<double, D> query, node * n) {
    return nullptr;
    // double projQuery = n->project(query);
    // double prediction = n->model.predict(projQuery));
    // if(n->isLeaf) {
    //     // Leaf search is a little complicated.
    //     // A complication is that even if n->project(data[i].first) == projQuery
    //     // it doesn't mean that query == data[i].first. 
    //     // Once leaf search finds such an i, resort to using equality across all dimensions to verify a match in a segment around i.

    //     // Leaf search is a 4 step process:
    //     // 1. Find leftBound in [start, end] such that n->project(data[leftBound].first) < projQuery
    //     // 2. Find rightBound in [start, end] such that n->project(data[rightBound].first) > projQuery
    //     // 3. Do binary search to find an index i such that n->project(data[i].first) == projQuery
    //     // 4. Do local search around i to see if any values in the dataset are strictly equal to query.
    //     uint p = clamp(
    //         floor(prediction * (n->end - n->start)),
    //         n->start, n->end
    //     );
    //     int c = compare(n->project(data[p].first), projQuery);
    //     int rightBound = p;
    //     int leftBound = p;
    //     if (c == 0) {
    //         return localSearch(p, n->start, n->end);
    //     } else if (c > 0) {
    //         // In this case, n->project(data[p].first) > projQuery, and so p is a rightBound
    //         // Now we have to find leftBound.
    //         // To do that, exponential search to the left.
    //         int gap = 1;
    //         while(c < 0 && leftBound >= n->start) {
    //             leftBound -= gap;
    //             gap *= 2;
    //             c = compare(n->project(data[leftBound].first), projQuery);
    //         }
    //         if (leftBound < n->start) {
    //             leftBound = n->start;
    //             c = compare(n->project(data[leftBound].first), projQuery);
    //         }
    //         if(c == 0) {
    //             return localSearch(leftBound, n->start, n->end);
    //         } else if (c < 0) {
    //             return nullptr;
    //         }
    //     } else {
    //         // In this case, n->project(data[p].first) < projQuery, and so p is a leftBound
    //         // Now we have to find a rightBound.
    //         // To do that, exponential search to the right.
    //         int gap = 1;
    //         while(c > 0 && rightBound <= n->end) {
    //             rightBound += gap;
    //             gap *= 2;
    //             c = compare(n->project(data[rightBound].first), projQuery);
    //         }
    //         if (rightBound > n->end) {
    //             rightBound = n->end;
    //             c = compare(n->project(data[rightBound].first, projQuery));
    //         }
    //         if(c == 0) {
    //             return localSearch(rightBound, n->start, n->end);
    //         } else if (c > 0) {
    //             return nullptr;
    //         }
    //     }
    // }
}

// sorts the vector of data between indices start and end according
// to the ordering induced by the linear functional proj
template <uint D, typename V>
void PiTree<D,V>::pairSort(node * n) {
    int length = n->end - n->start;
    vector<pair<double, datum>> paired; // TODO do this without copying
    for(int i = 0; i < length; i++) {
        paired.push_back(make_pair(
            n->project(data[i+n->start]),
            data[i+n->start]
        ));
    }
    sort(paired.begin(), paired.end());
    for(int i = 0; i < length; i++) {
        data[i+n->start] = paired[i].second;
    }
    return;
}

template <uint D, typename V>
void PiTree<D,V>::printSubTree(node * n, uint depth) {
    cout << string((int)depth * 2, ' ') << "- ";
    if(n->start == n->end) {
        cout << "NULL" << endl;
        return;
    }
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

double clamp(uint v, uint lb, uint ub) {
    return max(lb, min(ub, v));
}



