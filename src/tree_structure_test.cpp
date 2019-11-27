#include <array>
#include <cassert>
#include <cstdlib>
#include <vector>
#include <string>
#include <cstdio>
#include "pi_tree.h"

using namespace std;


void testTreeBuild() {
    const uint D = 3;
    const uint N = 30;
    vector<pair<array<double, D>, int>> data = vector<pair<array<double, D>, int>>(N);
    for(auto it = data.begin(); it != data.end(); it++) {
        array<double, D> datum;
        for(double * it2 = datum.begin(); it2 != datum.end(); it2++) {
            *it2 = static_cast<double>(rand()) / static_cast<double>(RAND_MAX) * 10;
        }
        int v = rand();
        *it = make_pair(datum, v);
    }
    PiTree<D,int> *t = new PiTree<D,int>(data, 3, 2);
    t->printTree();
}

int main(void) {
    testTreeBuild();
}