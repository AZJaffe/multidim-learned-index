#pragma once

// 10 is arbitrary
#define MAX_FANOUT 10

using namespace std;
#include <vector>
#include <iostream>
#include <iomanip>

struct linearModel {
public:
    double bias;
    double slope;
    linearModel(double bias, double slope): bias(bias), slope(slope) {}
    double predict(double x) {
        return bias + x * slope;
    }
    void print(void) {
        cout << "y = " << slope << "x + " << bias <<endl;
    }
};

// D is the dimension of the data
template <uint D>
class PTree {
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

    void pairSort(uint start, uint end, array<double, D> proj) {
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

// linearCdfRegressor builds a linear model from
// a stream of data points in sorted order.
// (1) Pass the (sorted) data points one at a time into add
// (2) Call fit()
// TODO(?): This class assumes data is made up of unique values, remove this assumption if necessary
class linearCdfRegressor {
public:
    double s_xy, s_x, s_xx;
    double n; // number of data points seen so far
    linearCdfRegressor() : s_xy(0), s_x(0), s_xx(0), n(0) {}
    void add(double d) {
        n++;
        s_xy += d * n;
        s_x += d;
        s_xx += d * d;
    }
    linearModel fit() {
        double s_y = (n+1) / 2;
        double slope = (s_xy - s_x*s_y) / (n*s_xx - s_x*s_x);
        return linearModel(
            (s_y - slope*s_x) / n,
            slope
        );
    }
};