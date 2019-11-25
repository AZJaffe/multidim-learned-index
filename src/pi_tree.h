#pragma once

// 10 is arbitrary
#define MAX_FANOUT 10

using namespace std;
#include <array>
#include <cassert>
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
    };
    node * root;

    node * buildSubTree(uint start, uint end, uint depth);
    void pairSort(uint start, uint end, const array<double, D> &proj);
    void printSubTree(node * n, uint depth);
    double dotProduct(const array<double, D> &v, const array<double, D> &w) {
        return inner_product(v.begin(), v.end(), w.begin(), 0);
    }

public:
    PiTree(vector<datum> &data, uint fanout, uint pageSize);
    void printTree();
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
    pairSort(start, end, n->proj);
    LinearCdfRegressor builder = LinearCdfRegressor();
    for (size_t i = 0; i < data.size(); i++) {
        builder.add(
            dotProduct(data[i].first, n->proj)
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
                dotProduct(data[i].first, n->proj)
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

// sorts the vector of data between indices start and end according
// to the ordering induced by the linear functional proj
template <uint D, typename V>
void PiTree<D,V>::pairSort(uint start, uint end, const array<double, D> &proj) {
    int length = end - start;
    vector<pair<int, datum>> paired; // TODO do this without copying
    for(int i = 0; i < length; i++) {
        paired.push_back(make_pair(
            dotProduct(data[i+start].first, proj),
            data[i+start]
        ));
    }
    std::sort(paired.begin(), paired.end());
    for(int i = 0; i < length; i++) {
        data[i+start] = paired[i].second;
    }
    return;
}

template <uint D, typename V>
void PiTree<D,V>::printTree() {
    printSubTree(root, 0);
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

