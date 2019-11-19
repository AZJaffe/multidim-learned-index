#include <array>
#include <cassert>
#include <vector>
#include "pi_tree.h"

using namespace std;


void testPairSort() {
    const uint D = 2;
    vector<array<double, D>> data;
    data.push_back(array<double, D>{2.0, 1.0});
    data.push_back(array<double, D>{1.0,2.0});
    data.push_back(array<double, D>{6.0,3.0});
    data.push_back(array<double, D>{5.0,4.0});
    data.push_back(array<double, D>{3.0,5.0});
    data.push_back(array<double, D>{4.0,6.0});
    array<double, D> proj = {1.0, 0.0};
    
    cout << "before:" << endl;
    for(auto n : data) {
        cout << "[" << n[0] << ", " << n[1] << "]" << endl;
    }
    cout << endl;
    pairSort<D>(data, 0, 5, proj);
    cout << "after:" << endl;
    for(auto n : data) {
        cout << "[" << n[0] << ", " << n[1] << "]" << endl;
    }
    cout << endl;

}

int main(void) {
    testPairSort();
}