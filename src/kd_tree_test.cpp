#include <array>
#include <cassert>
#include <vector>
#include <string>
#include <cstdio>
#include "kd_tree.h"

using namespace std;

void testRangeQuery() {
    const uint D = 2;
    auto data = vector<pair<array<double, D>, int>>();
    auto min = array<double, D>{1.0, 1.0};
    auto max = array<double, D>{2.0, 3.0};

    data.push_back(make_pair(array<double, D>{1.5, 1.5}, 0));
    data.push_back(make_pair(array<double, D>{1.8, 1.2}, 1));
    data.push_back(make_pair(array<double, D>{1.0, 3.0}, 2));
    data.push_back(make_pair(array<double, D>{1.0, 2.0}, 3));
    data.push_back(make_pair(array<double, D>{2.0, 3.0}, 4));
    data.push_back(make_pair(array<double, D>{2.0, 1.0}, 5));
    data.push_back(make_pair(array<double, D>{1.5, 2.1}, 6));
    data.push_back(make_pair(array<double, D>{1.5, 1.2}, 7));
    data.push_back(make_pair(array<double, D>{1.5, 2.9}, 8));
    data.push_back(make_pair(array<double, D>{1.2, 1.5}, 9));
    data.push_back(make_pair(array<double, D>{1.2, 1.5}, 10));
    data.push_back(make_pair(array<double, D>{1.1, 1.5}, 11));
    data.push_back(make_pair(array<double, D>{1.9, 1.5}, 12));

    // both out of bounds
    data.push_back(make_pair(array<double, D>{0.9, 3.9}, 12));
    data.push_back(make_pair(array<double, D>{0.9, 0.5}, 12));
    data.push_back(make_pair(array<double, D>{0.5, 10.4}, 12));
    data.push_back(make_pair(array<double, D>{0.4, 0.0}, 12));
    data.push_back(make_pair(array<double, D>{2.1, 3.9}, 12));
    data.push_back(make_pair(array<double, D>{2.1, 0.1}, 12));
    data.push_back(make_pair(array<double, D>{2.1, 0.8}, 12));

    // second out of bounds
    data.push_back(make_pair(array<double, D>{1.2, 3.9}, 13));
    data.push_back(make_pair(array<double, D>{1.8, 0.5}, 13));
    data.push_back(make_pair(array<double, D>{1.2, 10.4}, 13));
    data.push_back(make_pair(array<double, D>{1.0, 0.0}, 13));
    data.push_back(make_pair(array<double, D>{2.0, 0.0}, 13));

    // first out of bounds
    data.push_back(make_pair(array<double, D>{0.5, 10.4}, 13));
    data.push_back(make_pair(array<double, D>{0.4, 0.0}, 13));
    data.push_back(make_pair(array<double, D>{2.1, 3.9}, 13));
    data.push_back(make_pair(array<double, D>{2.1, 0.1}, 13));
    data.push_back(make_pair(array<double, D>{2.1, 0.8}, 13));

    auto t = KdTree<D, int>(data);
    vector<pair<array<double, D>, int>> ret = t.rangeQuery(min, max);
    sort(ret.begin(), ret.end(), [](const pair<array<double, D>, int> &a, const pair<array<double, D>, int> &b) { return a.second < b.second; });
    assert(ret.size() == 13);
    for(int i = 0; i < (int)ret.size(); i++) {
        assert(ret[i].second == i);
    }
}

int main(void) {
    testRangeQuery();
}