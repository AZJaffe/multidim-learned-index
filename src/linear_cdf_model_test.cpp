#include <cassert>
#include <vector>
#include "linear_cdf_model.h"
using namespace std;

void test1(void) {
    vector<double> data = {5.0,7.0,9.0,11.0,13.0,15.0,17.0,19.0,21.0,23.0,25.0,27.0};
    linearCdfModelBuilder builder = linearCdfModelBuilder();
    for(uint i = 0; i < data.size(); i++) {
        builder.add(data[i]);
    }
    linearModel model = builder.build();
    cout << "test1 model: " << endl;
    model.print();
    cout << (model.predict(7.0) * data.size()) << " should equal 2" << endl;
    cout << (model.predict(13.0) * data.size()) << " should equal 5" << endl;
}

void test2(void) {
    vector<double> data = {1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0};
    linearCdfModelBuilder builder = linearCdfModelBuilder();
    for(uint i = 0; i < data.size(); i++) {
        builder.add(data[i]);
    }
    linearModel model = builder.build();
    cout << "test2 model: " << endl;
    model.print();
    cout << (model.predict(4.5) * data.size()) << " should equal 4.5" << endl;
    cout << (model.predict(7.2) * data.size()) << " should equal 7.2" << endl;
}

int main(void) {
    test1();
    test2();
}