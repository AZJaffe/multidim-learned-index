#include <iostream>
#include "pca_solver.h"

using namespace std;

int main() {
    const int dim = 2;
    const int numData = 3;
    auto data = vector<pair<array<double, dim>, int>>(numData);
    // Solve matrix [[1,2]
    //               [2,4]
    //               [10,20]] 
    data[0] = make_pair(array<double, dim>{1,2}, -1);
    data[1] = make_pair(array<double, dim>{2,4}, -1);
    data[2] = make_pair(array<double, dim>{10,20}, -1);

    PCASolver<dim> p(false);
    p.load_data(data);
    
    p.solve();

    auto eigenvalues = p.get_eigenvalues();
    cout << "Eigen values: " << endl;
    for (auto it = eigenvalues.begin(); it != eigenvalues.end(); ++it) {
        cout << *it << endl;
    }
    cout << "=======================" << endl;
	cout << "Eigen vectors: " << endl;
    for (int i = 0; i < eigenvalues.size(); ++i) {
        auto eigenvector = p.get_eigenvector(i);
        for (auto it = eigenvector.begin(); it != eigenvector.end(); ++it) {
            cout << *it << " , ";
        }
        cout << endl;
    }
    cout << "=======================" << endl;
	cout << "Principal vectors: " << endl;
    for (int i = 0; i < eigenvalues.size(); ++i) {
        auto principalvector = p.get_principal(i);
        for (auto it = principalvector.begin(); it != principalvector.end(); ++it) {
            cout << *it << " , ";
        }
        cout << endl;
    }
 
    return 0;
}