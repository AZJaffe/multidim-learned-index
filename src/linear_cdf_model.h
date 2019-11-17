#pragma once

using namespace std;
#include <vector>
#include <iostream>

class linearModel {
    public:
    double bias;
    double slope;
    linearModel(double bias, double slope): bias(bias), slope(slope) {}
    double predict(double x) {
        return bias + x * slope;
    }
    void print() {
        cout << "slope: " << slope << endl << "bias: " << bias <<endl;
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
    double slope = (n*s_xy - s_x*s_y) / (n*s_xx - s_x*s_x);
    return linearModel(
        (s_y - slope*s_x) / n,
        slope
    );
}