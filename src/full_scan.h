#pragma once

#include <vector>
#include <array>
#include "util.h"

using namespace std;

template <uint D, typename V>
class FullScan {
    typedef pair<array<double, D>, V> datum;
    vector<datum> & data;
public:
    FullScan(vector<datum> & data) : data(data) {};
    vector<datum> rangeQuery(array<double, D> min, array<double, D> max) {
        vector<datum> ret = vector<datum>();
        for(auto it = data.begin(); it != data.end(); it++) {
            if (withinRange<D>(it->first, min, max)) {
                ret.push_back(*it);
            }
        }
        return ret;
    }
};