#pragma once

#include <cmath>

#ifndef EPSILON
#define EPSILON 1e-8
#endif

int compare(double a, double b) {
    if (abs(a - b) < EPSILON) {
        return 0;
    }
    return a < b ? -1 : 1;
}

template <uint D>
bool withinRange(array<double, D> p, array<double, D> min, array<double, D> max) {
    for(uint d = 0; d < D; d++) {
        if (p[d] < min[d] || p[d] > max[d]) {
            return false;
        }
    }
    return true;
}