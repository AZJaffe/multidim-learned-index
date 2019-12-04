#pragma once

#ifdef TRACE
#  define TPRINT(x) cout << x << endl
#  define IFTRACE if(1)
#else
#  define TPRINT(x) do {} while (0)
#  define IFTRACE if(0)
#endif

using namespace std;
#include <vector>
#include <iostream>
#include <iomanip>
#include <math.h>

struct LinearModel {
public:
    double bias;
    double slope;
    LinearModel(double bias, double slope): bias(bias), slope(slope) {}
    double predict(double x) {
        return bias + x * slope;
    }
    void print(void) {
        cout << "y = " << slope << "x + " << bias <<endl;
    }
};

// LinearCdfRegressor builds a linear model from
// a stream of data points in sorted order.
// (1) Pass the (sorted) data points one at a time into add
// (2) Call fit()
// TODO(?): This class assumes data is made up of unique values, remove this assumption if necessary
class LinearCdfRegressor {
public:
    double s_xy, s_x, s_xx;
    double n; // number of data points seen so far
    double max;
    LinearCdfRegressor() : s_xy(0), s_x(0), s_xx(0), n(0), max(0) {}
    void add(double d) {
        assert(d >= max);
        IFTRACE max = d;
        n++;
        s_xy += d * n;
        s_x += d;
        s_xx += d * d;
    }
    LinearModel fit() {
        assert(n != 0);
        double s_y = (n+1) / 2;
        double slope = (s_xy - s_x*s_y) / (n*s_xx - s_x*s_x);
        if(isnan(slope)) {
            return LinearModel(s_x, 0);
        }
        return LinearModel(
            (s_y - slope*s_x) / n,
            slope
        );
    }
};