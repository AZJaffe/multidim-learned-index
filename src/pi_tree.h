#pragma once

// 10 is arbitrary
#define MAX_FANOUT 10

using namespace std;
#include <vector>
#include <iostream>
#include <iomanip>

// D is the dimension of the data
template <uint D>
class PiTree {
    vector<array<double, D>> data; // Data is an array of arrays of doubles of size D
    uint fanout;
    uint height; // e.g. a tree where the root is a leaf has height 1

    struct node {
        node children[MAX_FANOUT];
        array<double, D> projection;
        linearModel model;
        bool isLeaf;
        // start and end will only be used if isLeaf is true.
        // they correspond to the starting and ending indecies of the data this leaf indexes
        int start; 
        int end;
    };
    node * root;

    public:
    PTree(std::vector<std::array<double, D>> data, uint fanout) :
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
void pairSort(vector<array<double, D>> data, uint start, uint end, array<double, D> proj) {
    int length = end - start;
    pair<int, array<double, D>> * paired = new pair<int, array<double, D>>[length];
    for(int i = 0; i < length; i++) {
        paired[i].first = dotProduct(data[i+start], proj);
        paired[i].second = data[i+start];
    }
    std::sort(paired->begin(), paired->end());
    for(int i = 0; i < length; i++) {
        data[i+start] = paired[i].second;
    }
    delete[] paired;
    return;
}
