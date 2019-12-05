#pragma once

#ifdef TRACE
#  define TPRINT(x) cout << x << endl
#  define IFTRACE if(1)
#else
#  define TPRINT(x) do {} while (0)
#  define IFTRACE if(0)
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
#include "exponential_search.h"

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
            int prediction = floor(model.predict(d) * (end - start)) + start;
            return max(start, min(prediction, end - 1));
        };
    };
    node * root;

    node * buildSubTree(uint start, uint end, uint depth);
    void pairSort(node & n);
    void printSubTree(node * n, uint depth, bool printData = false);
    datum * lookup(array<double, D> query, node * n);
    void rangeQuery(vector<datum> &ret, array<double, D> min, array<double, D> max, node * n);

public:
    PiTree(vector<datum> &data, uint fanout, uint pageSize);
    datum * lookup(array<double, D> query) {
        return lookup(query, root);
    }
    vector<datum> rangeQuery(array<double, D> min, array<double, D> max) {
        IFTRACE for(uint i = 0; i < D; i++) assert(min[i] <= max[i]);
        assert(root != nullptr);
        vector<datum> ret;
        rangeQuery(ret, min, max, root);
        return ret;
    }
    void printTree(bool printData = false) {
        printSubTree(root, 0, printData);
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
        int predictedMinIndex = n->getIndex(minProjection);
        int predictedMaxIndex = n->getIndex(maxProjection);
        // The predicted indices could be off. Have to do exponential search to find the actual minIndex,maxIndex to search

        // 2 ways of doing this
        // first is the get lower bound then iterate and check every stage
        // second is to get lower and upper bound, then iterate between (without checking)
        auto start = exponentialSearchLowerBound(data.begin() + n->start, data.begin() + n->end, data.begin() + predictedMinIndex, minProjection,
            [n](datum &d, double p) { return n->project(d.first) < p; });
        auto end = exponentialSearchUpperBound(data.begin() + n->start, data.begin() + n->end, data.begin() + predictedMaxIndex, maxProjection,
            [n](double p, datum &d) { return p < n->project(d.first); });

        TPRINT("Range scanning node with range [" << n->start << ", " << n->end << ") on subrange [" << start - data.begin() << ", " << end - data.begin() << ")");
        for(auto it = start; it < end; it++) {
            bool withinBounds = true;
            for(uint d = 0; d < D; d++) {
                if (it->first[d] < min[d] || it->first[d] > max[d]) {
                    withinBounds = false;
                    break;
                }
            }
            if (withinBounds) {
                ret.push_back(*it);
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
        double projQuery = n->project(query);
        int prediction = n->getIndex(projQuery);
        int p = max(n->start, min(prediction, n->end));
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
    int childIndex = n->getChildIndex(projQuery);
    return lookup(query, n->children[childIndex]);
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
    cout << "isLeaf=" << n->isLeaf;
    cout << endl;
    if(n->isLeaf && printData) {
        for(int i = n->start; i < n->end; i++) {
            cout << string((int)depth * 2 + 2, ' ') << "- {[";
            for(uint d = 0; d < D-1; d++) {
                cout << data[i].first[d] << ", ";
            }
            cout << data[i].first[D-1] << "], ";
            cout << data[i].second << "}" << endl;
        }
    }
    for(uint i = 0; i < n->children.size(); i++) {
        printSubTree(n->children[i], depth+1, printData);
    }
}
