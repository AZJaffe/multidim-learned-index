#include <array>
#include <cassert>
#include <vector>
#include <string>
#include <cstdio>
#include "pi_tree.h"

using namespace std;


void testPairSort() {
    const uint D = 2;
    vector<pair<array<double, D>, int>> data;
    data.push_back(make_pair(array<double, D>{2.0, 1.0}, 1));
    data.push_back(make_pair(array<double, D>{1.0, 2.0}, 2));
    data.push_back(make_pair(array<double, D>{6.0, 3.0}, 3));
    data.push_back(make_pair(array<double, D>{5.0, 4.0}, 4));
    data.push_back(make_pair(array<double, D>{3.0, 5.0}, 5));
    data.push_back(make_pair(array<double, D>{4.0, 6.0}, 6));
    
    cout << "before:" << endl;
    for(auto n : data) {
        printf("{(%1.0f, %1.0f), %d} ", n.first[0], n.first[1], n.second);
    }
    cout << endl;
    PiTree<D,int> *t = new PiTree<D,int>(data, 10);
    (void)*t;
    cout << "after:" << endl;
    for(auto n : data) {
        printf("{(%1.0f, %1.0f), %d} ", n.first[0], n.first[1], n.second);
    }
    cout << endl;

}

int main(void) {
    testPairSort();
}