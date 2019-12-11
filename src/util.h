#pragma once

#include <cmath>
#include <fstream>
#include <string>
#include <vector>
// #include <iterator>
// #include <algorithm>
#include <boost/algorithm/string.hpp>

using namespace std;

#ifndef EPSILON
#define EPSILON 1e-15
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

class CSVReader {
    string fileName;
    string delimeter;

public:
    CSVReader(string filename, string delm=","):
        fileName(filename), delimeter(delm) {}
    
    vector<vector<string>> getData();
};

vector<vector<string>> CSVReader::getData() {
    ifstream file(fileName);
    vector<vector<string>> dataList;
    string line = "";

    while(getline(file, line)) {
        vector<string> tmp;
        boost::algorithm::split(tmp, line, boost::is_any_of(delimeter));
        dataList.push_back(tmp);
    }

    file.close();
    return dataList;
}