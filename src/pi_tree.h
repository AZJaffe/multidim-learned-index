#pragma once

#ifdef DEBUG
#  define DPRINT(x) cout << x << endl
#  define IFDEBUG if(1)
#else
#  define DPRINT(x) do {} while (0)
#  define IFDEBUG if(0)
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

// TODO? make a better double comparison
int compare(double a, double b) {
    if (abs(a - b) < numeric_limits<double>::epsilon()) {
        return 0;
    }
    return a < b ? -1 : 1;
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
        double project(array<double, D> &d) {
            return inner_product(d.begin(), d.end(), proj.begin(), 0.0);
        };
        int getChildIndex(double d) {
            assert(!isLeaf);
            int prediction = floor(model.predict(d) * children.size());
            return max(0, min(prediction, (int)children.size() - 1));
        };
        int getIndex(double d) {
            assert(isLeaf);
            int prediction = floor(model.predict(d) * (end - start));
            return max(start, min(prediction, end - 1));
        };
    };
    node * root;

    node * buildSubTree(uint start, uint end, uint depth);
    void pairSort(node & n);
    void printSubTree(node * n, uint depth);
    datum * lookup(array<double, D> query, node * n);
    datum * searchLeaf(array<double, D> query, node * n);
    datum * localSearch(array<double, D> query, double projQuery, node * n, uint start);
    void rangeQuery(vector<datum> &ret, array<double, D> min, array<double, D> max, node * n);

public:
    PiTree(vector<datum> &data, uint fanout, uint pageSize);
    datum * lookup(array<double, D> query) {
        return lookup(query, root);
    }
    vector<datum> rangeQuery(array<double, D> min, array<double, D> max) {
        IFDEBUG for(uint i = 0; i < D; i++) assert(min[i] <= max[i]);
        assert(root != nullptr);
        vector<datum> ret;
        rangeQuery(ret, min, max, root);
        return ret;
    }
    void printTree() {
        printSubTree(root, 0);
    }
};

template <uint D, typename V>
PiTree<D,V>::PiTree(vector<datum> &data, uint fanout, uint pageSize) :
data(data), fanout(fanout), pageSize(pageSize) {
    assert(fanout > 0);
    assert(pageSize > 1);
    root = buildSubTree(0, data.size(), 0);
}

template <uint D, typename V>
typename PiTree<D,V>::node * PiTree<D,V>::buildSubTree(uint start, uint end, uint depth) {
    DPRINT("Building subtree with start=" << start << " end=" << end);
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
    DPRINT("regressor=(" << n->model.slope << "x + " << n->model.bias << ")");
    n->isLeaf = (end - start < pageSize);
    if (!n->isLeaf) {
        uint childStart = start;
        double childMaxVal = 1.0 / (double)fanout;
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
                if (n->children.size() == fanout - 1) {
                    childMaxVal = numeric_limits<double>::max();
                } else {
                    childMaxVal += maxValIncrement;
                }
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

template<uint D, typename V>
void PiTree<D,V>::rangeQuery(vector<typename PiTree<D,V>::datum> &ret, array<double, D> min, array<double, D> max, node * n) {
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

    if(n->isLeaf) {
        int minIndex = n->getIndex(minProjection);
        int maxIndex = n->getIndex(maxProjection);
        for(int i = minIndex; i <= maxIndex; i++) {
            bool withinBounds = true;
            for(uint d = 0; d < D; d++) {
                if (data[i].first[d] < min[d] || data[i].first[d] > max[d]) {
                    withinBounds = false;
                    break;
                }
            }
            if (withinBounds) {
                ret.push_back(data[i]);
            }
        }
        return;
    } else {
        int minChildIndex = n->getChildIndex(minProjection);
        int maxChildIndex = n->getChildIndex(maxProjection);

        for(int i = minChildIndex; i <= maxChildIndex; i++) {
            rangeQuery(ret, min, max, n->children[i]);
        }
        return;
    }
}

template <uint D, typename V>
typename PiTree<D,V>::datum * PiTree<D,V>::lookup(array<double, D> query, node * n) {
    if(n->isLeaf) {
        return searchLeaf(query, n);   
    }
    double projQuery = n->project(query);
    int childIndex = n->getChildIndex(projQuery);
    return lookup(query, n->children[childIndex]);
}

template <uint D, typename V>
typename PiTree<D,V>::datum * PiTree<D,V>::localSearch(array<double, D> query, double projQuery, node * n, uint start) {
    int l = start;
    while(l < n->end && compare(projQuery, n->project(data[l].first)) == 0) {
        bool equal = true;
        for(uint i = 0; i < D; i++) {
            if (compare(query[i], data[l].first[i]) != 0) {
                equal = false;
                break;
            }
        }
        if (equal) {
            return &data[l];
        }
        l++;
    }
    l = start;
    while(l >= n->start && compare(projQuery, n->project(data[l].first) == 0)) {
        bool equal = true;
        for(uint i = 0; i < D; i++) {
            if (compare(query[i], data[l].first[i]) != 0) {
                equal = false;
                break;
            }
        }
        if (equal) {
            return &data[l];
        }
        l--;
    }
    return nullptr;
}


// Leaf search is a little complicated.
// Part of the complexity arises from the fact that even if n->project(data[i].first) == projQuery
// it doesn't mean that query == data[i].first. 
// Once leaf search finds such an i, call localSearch which will use equality across all dimensions to verify a match in a segment around i.

// Leaf search is a 4 step process:
// 1. Find leftBound in [start, end] such that n->project(data[leftBound]) < projQuery, using exponential search
// 2. Find rightBound in [start, end] such that n->project(data[rightBound]) > projQuery, using exponential search
// 3. Do binary search to find an index i such that n->project(data[i].first) == projQuery
// 4. Do local search around i to see if any values in the dataset are strictly equal to query.

// Either step 1 or 2 will run, not both.
// If an index i such that n->project(data[i].first) == projQuery is found, then immediately do local search
template <uint D, typename V>
typename PiTree<D,V>::datum * PiTree<D,V>::searchLeaf(array<double, D> query, node * n) {
    assert(n->isLeaf);
    double projQuery = n->project(query);
    int prediction = n->getIndex(projQuery);
    int p = max(n->start, min(prediction, n->end));
    int c = compare(n->project(data[p].first), projQuery);
    int rightBound = p;
    int leftBound = p;
    if (c == 0) {
        return localSearch(query, projQuery, n, p);
    } else if (c > 0) {
        // In this case, n->project(data[p]) > projQuery, and so p is a rightBound
        // Now we have to find leftBound.
        // To do that, exponential search to the left.
        int gap = 1;
        while (c > 0 && leftBound > n->start) {
            rightBound = leftBound;
            leftBound = max(leftBound - gap, n->start);
            gap *= 2;
            c = compare(n->project(data[leftBound].first), projQuery);
        }
        if (c == 0) {
            return localSearch(query, projQuery, n, leftBound);
        } else if (c > 0) { // => leftBound = n->start
            return nullptr;
        }
    } else {
        // In this case, n->project(data[p]) < projQuery, and so p is a leftBound
        // Now we have to find a rightBound.
        // To do that, exponential search to the right.
        int gap = 1;
        while(c < 0 && rightBound < n->end - 1) {
            leftBound = rightBound;
            rightBound = min(rightBound + gap, n->end - 1);
            gap *= 2;
            c = compare(n->project(data[rightBound].first), projQuery);
        }
        if(c == 0) {
            return localSearch(query, projQuery, n, rightBound);
        } else if (c < 0) { // => rightBound = n->end - 1
            return nullptr;
        }
    }
    auto it = lower_bound(data.begin() + leftBound, data.begin() + rightBound, projQuery,
        [n](datum &d, double p) { return compare(n->project(d.first), p) < 0; } );
    uint index = distance(data.begin(), it);
    return localSearch(query, projQuery, n, index);
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

