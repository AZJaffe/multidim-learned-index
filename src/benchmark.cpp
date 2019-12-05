#include <array>
#include <cassert>
#include <vector>
#include <string>
#include <cstdio>
#include "pi_tree.h"

using namespace std;

// not really a benchmark yet, but it does slam the tree pretty hard!
void benchmarkRangeQuery() {
    const uint D = 2;
    const uint N = 1e7;
    auto data = vector<pair<array<double, D>, int>>(N);

    for(auto itd = data.begin(); itd != data.end(); itd++) {
        array<double, D> k;
        for(auto it = k.begin(); it != k.end(); it++) {
            *it = static_cast<double>(rand()) / static_cast<double>(RAND_MAX) * 10.0;
        }
        int v = itd - data.begin();
        *itd = make_pair(k, v);
    }
    PiTree<D,int> *t = new PiTree<D,int>(data, 1e3, 5e3);
    
    const uint numQueries = 1000;
    for(uint i = 0; i < numQueries; i++) {
        array<double, D> min;
        array<double, D> max;
        for(uint d = 0; d < D; d++) {
            min[d] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX) * 10.0;
            max[d] = min[d] + (static_cast<double>(rand()) / static_cast<double>(RAND_MAX) * (10.0 - min[d]));
        }
        auto ret = t->rangeQuery(min, max);
        cout << "Size of result set: " << ret.size() << endl;
    }
}

int main(void) {
    benchmarkRangeQuery();
}