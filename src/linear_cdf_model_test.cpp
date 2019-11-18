#include <cassert>
#include <vector>
#include "linear_cdf_model.h"
using namespace std;

void test1(void) {
    vector<double> data = {5.0,7.0,9.0,11.0,13.0,15.0,17.0,19.0,21.0,23.0};
    linearModel model = fitLinearCdf(data);
    cout << "test1 model: " << endl;
    model.print();
    cout << (model.predict(7.0) * data.size()) << " should equal 2" << endl;
    cout << (model.predict(13.0) * data.size()) << " should equal 5" << endl;
}

int main(void) {
    test1();   
}