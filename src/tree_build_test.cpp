#include <array>
#include <cassert>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdio>
#include "pi_tree.h"

using namespace std;

void testTreeBuild() {
    const uint D = 3;
    const uint N = 40;
    auto data = vector<pair<array<double, D>, int>>(N);
    for(auto itd = data.begin(); itd != data.end(); itd++) {
        array<double, D> k;
        for(auto it = k.begin(); it != k.end(); it++) {
            *it = static_cast<double>(rand()) / static_cast<double>(RAND_MAX) * 10.0;
        }
        int v = itd - data.begin();
        *itd = make_pair(k, v);
    }
    PiTree<D,int> *t = new PiTree<D,int>(data, 3, 5);
    t->printTree();
}

int main(void) {
    testTreeBuild();
}