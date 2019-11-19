#pragma once

// 10 is arbitrary
#define MAX_FANOUT 10

using namespace std;
#include <array>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>
#include "linear_cdf_regressor.h"

// D is the dimension of the data
template <uint D>
class PiTree {
    vector<array<double, D>> data; // Data is an array of arrays of doubles of size D
    uint fanout;
    uint height; // e.g. a tree where the root is a leaf has height 1

    struct node {
        node children[MAX_FANOUT];
        array<double, D> projection;
        LinearModel model;
        bool isLeaf;
        // start and end will only be used if isLeaf is true.
        // they correspond to the starting and ending indecies of the data this leaf indexes
        int start; 
        int end;
    };
    node * root;

    public:
    PiTree(std::vector<std::array<double, D>> data, uint fanout) :
    fanout(fanout), height(1), data(data) {
        root = new node();
        for(int i = 1; i < D; i++) {
            root->projection[i] = 0;
        }
        root->projection[0] = 1;
        pairSort(0, data.size(), root->projection);
    }
};

template <uint D>
void pairSort(vector<array<double, D>> &data, uint start, uint end, array<double, D> proj) {
    int length = end - start + 1;
    vector<pair<int, array<double, D>>> paired;
    for(int i = 0; i < length; i++) {
        paired.push_back(make_pair(
            inner_product(data[i+start].begin(), data[i+end].end(), proj.begin(), 0),
            data[i+start]
        ));
    }
    std::sort(paired.begin(), paired.end());
    for(int i = 0; i < length; i++) {
        data[i+start] = paired[i].second;
    }
    return;
}
