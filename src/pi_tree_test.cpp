#include <array>
#include <cassert>
#include <vector>
#include <string>
#include <cstdio>
#include "pi_tree.h"

using namespace std;


void testPairSort(bool debug) {
    const uint D = 2;
    vector<pair<array<double, D>, int>> data;
    data.push_back(make_pair(array<double, D>{2.0, 1.0}, 1));
    data.push_back(make_pair(array<double, D>{1.0, 2.0}, 2));
    data.push_back(make_pair(array<double, D>{6.0, 3.0}, 3));
    data.push_back(make_pair(array<double, D>{5.0, 4.0}, 4));
    data.push_back(make_pair(array<double, D>{3.0, 5.0}, 5));
    data.push_back(make_pair(array<double, D>{4.0, 6.0}, 6));
    
    if (debug) {
        cout << "before:" << endl;
        for(auto n : data) {
            printf("{(%1.0f, %1.0f), %d} ", n.first[0], n.first[1], n.second);
        }
        cout << endl;
    }
    PiTree<D,int> *t = new PiTree<D,int>(data, 10, 100);
    (void)*t;

    if (debug) {
        cout << "after:" << endl;
        for(auto n : data) {
            printf("{(%1.0f, %1.0f), %d} ", n.first[0], n.first[1], n.second);
        }
        cout << endl;
        t->printTree();
    }
    assert(data[0].second == 2);
    assert(data[1].second == 1);
    assert(data[2].second == 5);
    assert(data[3].second == 6);
    assert(data[4].second == 4);
    assert(data[5].second == 3);
    assert(data.size() == 6);
}

void testTreeBuild() {
    const uint D = 2;
    vector<pair<array<double, D>, int>> data;
    data.push_back(make_pair(array<double, D>{2.0, 1.0}, 1));
    data.push_back(make_pair(array<double, D>{1.0, 2.0}, 2));
    data.push_back(make_pair(array<double, D>{6.0, 3.0}, 3));
    data.push_back(make_pair(array<double, D>{5.0, 4.0}, 4));
    data.push_back(make_pair(array<double, D>{3.0, 5.0}, 5));
    data.push_back(make_pair(array<double, D>{4.0, 6.0}, 6));
}

void testTreeBuild2() {
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
    PiTree<D,int> *t = new PiTree<D,int>(data, 3, 10);
    t->printTree();
}

void testPointQuery() {
    const uint D = 1;
    const uint N = 1000;
    auto data = vector<pair<array<double, D>, int>>(N);
    for(auto it = data.begin(); it != data.end(); it++) {
        array<double, D> k;
        k[0] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX) * 10.0;
        int v = it - data.begin();
        *it = make_pair(k, v);
    }
    PiTree<D,int> *t = new PiTree<D,int>(data, 1, 1000); // one level tree since fanout = N
    pair<array<double, D>, int> * d = t->lookup(data[12].first);
    assert(d != nullptr && d->second == data[12].second);
}

int main(void) {
    // testPairSort(true);
    // testTreeBuild();
    // testTreeBuild2();
    testPointQuery();
}