#pragma once

// 10 is arbitrary
#define MAX_FANOUT 10

using namespace std;
#include <vector>
#include <iostream>

// D is the dimension of the data
template <uint D>
class PTree {
    double data[]; // For now, index one array instead of leaves pointing to data pages
    uint fanout;
    uint height; // e.g. a tree where the root is leaf has height 1

    struct node {
        node * [MAX_FANOUT] children;
        double[D] projection;
        linearModel model;
        isLeaf bool;
        // start and end will only be used if isLeaf is true.
        // they correspond to the starting and ending indecies of the data this leaf indexes
        int start; 
        int end;
    };
    public:
    node * root;
    PTree(double data[], uint fanout, uint height) :
    fanout(fanout), height(height) {
        
    }
};

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

// Assuming data is made up of unique values
// TODO? remove this assumption
linearModel fitLinearCdf(vector<double> data) {
    double n = data.size();
    double s_xy = 0, s_x = 0, s_xx = 0;
    double s_y = n * (n+1) / 2;
    for(int i = 0; i < n; i++) {
        s_xy += data[i] * (i+1);
        s_x += data[i];
        s_xx += data[i] * data[i];
    }
    double slope = (s_xy - s_x*s_y/n) / (n*s_xx - s_x*s_x);
    return linearModel(
        (s_y/n - slope*s_x) / n,
        slope
    );
}