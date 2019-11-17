#include <cassert>
#include <vector>
#include "linear_cdf_model.h"
using namespace std;

int main(void) {
    vector<double> data = {1.0,2.0,3.0,4.0,5.0,6.0,7.0};
    linearModel model = fitLinearCdf(data);
    assert(model.bias == 0);
    assert(model.slope == 1);
}