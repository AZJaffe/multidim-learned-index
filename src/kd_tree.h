#pragma once


#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <vector>
#include "util.h"

using namespace std;

// D is the dimension of the key of the data, V is the type of the data values
template <uint D, typename V>
class KdTree {
    typedef pair<array<double, D>, V> datum;

    struct node {
        datum d;
        node * left;
        node * right;
        node(datum & d) : d(d), left(nullptr), right(nullptr) {};
        size_t memorySize() {
            size_t s = sizeof(datum);
            s += 2 * sizeof(node);
            if (left != nullptr)
                s += left->memorySize();
            if (right != nullptr)
                s += right->memorySize();
            return s;
        }
    };

    struct nodeCmp {
        nodeCmp(size_t i) : i(i) {}
        size_t i;
        bool operator()(const node& n1, const node& n2)
        {
            return n1.d.first[i] < n2.d.first[i];
        }
    };
    node * root;
    vector<node> nodes;

    template <typename iterator>
    node * buildTree(iterator begin, iterator end, size_t i) {
        if (end <= begin)
            return nullptr;
        iterator n = begin + (end - begin)/2;
        nth_element(begin, n, end, nodeCmp(i));
        i = (i + 1) % D;
        n->left = buildTree(begin, n, i);
        n->right = buildTree(n + 1, end, i);
        return &(*n);
    }

    void rangeQuery(vector<datum> & ret, array<double, D> min, array<double, D> max, node * n, size_t i) {
        if (n == nullptr) {
            return;
        }

        if (withinRange<D>(n->d.first, min, max)) {
            ret.push_back(n->d);
        }

        size_t j = (i + 1) % D;
        if (n->d.first[i] >= min[i]) {
            rangeQuery(ret, min, max, n->left, j);
        }
        if (n->d.first[i] <= max[i]) {
            rangeQuery(ret, min, max, n->right, j);
        }
    }

public:
    KdTree(vector<datum> & d) {
        nodes.reserve(d.size());
        for (auto it = d.begin(); it != d.end(); it++) {
            nodes.emplace_back(*it);
        }
        root = buildTree(nodes.begin(), nodes.end(), 0);
    }
    ~KdTree() {}
    
    vector<datum> rangeQuery(array<double, D> min, array<double, D> max) {
        assert(root != nullptr);
        vector<datum> ret;
        rangeQuery(ret, min, max, root, 0);
        return ret;
    }

    size_t memorySize() {
        size_t s = sizeof(KdTree<D,V>);
        s += root->memorySize();
        return s;
    }
};
