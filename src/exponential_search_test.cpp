#include <cassert>
#include <vector>
#include "exponential_search.h"
using namespace std;

void exponentialSearchLowerBoundTest(void) {
    auto comp = [](int a, int b) { return a < b; };
    vector<int> data = { 1, 1, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 6 };

    for(uint i = 0; i < data.size(); i++) {
        auto it = exponentialSearchLowerBound(data.begin(), data.end(), data.begin() + i, 3, comp);
        assert(it - data.begin() == 3);
    }

    auto it = exponentialSearchLowerBound(data.begin(), data.end(), data.begin() + 7, 1, comp);
    assert(it - data.begin() == 0);

    it = exponentialSearchLowerBound(data.begin(), data.end(), data.begin() + 0, 6, comp);
    assert(it - data.begin() == 12);
    
    it = exponentialSearchLowerBound(data.begin(), data.end(), data.begin() + 7, 4, comp);
    assert(it - data.begin() == 7);
}

void exponentialSearchUpperBoundTest(void) {
    auto comp = [](int a, int b) { return a < b; };
    vector<int> data = { 1, 1, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 6 };

    for(uint i = 0; i < data.size(); i++) {
        auto it = exponentialSearchUpperBound(data.begin(), data.end(), data.begin() + i, 3, comp);
        assert(it - data.begin() == 7);
    }

    auto it = exponentialSearchUpperBound(data.begin(), data.end(), data.begin() + 7, 1, comp);
    assert(it - data.begin() == 2);

    it = exponentialSearchUpperBound(data.begin(), data.end(), data.begin() + 0, 6, comp);
    assert(it - data.begin() == 13);
    
    it = exponentialSearchUpperBound(data.begin(), data.end(), data.begin() + 7, 4, comp);
    assert(it - data.begin() == 10);
}

int main(void) {
    exponentialSearchLowerBoundTest();
    exponentialSearchUpperBoundTest();
}