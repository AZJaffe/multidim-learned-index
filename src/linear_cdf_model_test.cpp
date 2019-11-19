#include <cassert>
#include <vector>
#include "linear_cdf_model.h"
using namespace std;

void test1(void) {
    vector<double> data = {5.0,7.0,9.0,11.0,13.0,15.0,17.0,19.0,21.0,23.0,25.0,27.0};
    linearCdfRegressor builder = linearCdfRegressor();
    for(uint i = 0; i < data.size(); i++) {
        builder.add(data[i]);
    }
    linearModel model = builder.fit();
    cout << "test1 model: " << endl;
    model.print();
    cout << (model.predict(15.0)) << " should equal 0.5" << endl;
    cout << (model.predict(3.0)) << " should equal 0.0" << endl;
}

int main(void) {
    test1();
}