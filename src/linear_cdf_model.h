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
    double * data; // For now, index one array instead of leaves pointing to data pages
    uint fanout;
    uint height; // e.g. a tree where the root is leaf has height 1

    struct node {
        node children[MAX_FANOUT];
        double projection[D];
        linearModel model;
        bool isLeaf;
        // start and end will only be used if isLeaf is true.
        // they correspond to the starting and ending indecies of the data this leaf indexes
        int start; 
        int end;
    };
    public:
    node * root;
    PTree(double data[], uint fanout, uint height) :
    fanout(fanout), height(height), data(data) {

    }
};

// linearCdfRegressor builds a linear model from
// a stream of data points in sorted order.
// (1) Pass the (sorted) data points one at a time into add
// (2) Call build()
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
    linearModel build() {
        double s_y = n * (n+1) / 2;
        double slope = (n*s_xy - s_x*s_y) / (n*s_xx - s_x*s_x);
        return linearModel(
            (s_y - slope*s_x) / (n * n),
            slope / n
        );
    }
};
