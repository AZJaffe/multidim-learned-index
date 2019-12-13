#include <array>
#include <vector>
#include "pca.h"

using namespace std;

template <uint D>
class PCASolver {
public:
    stats::pca pca;

    PCASolver(bool do_bootstrap) {
        pca = stats::pca(D);
        pca.set_solver("dc");
        pca.set_do_normalize(true);
        pca.set_do_bootstrap(do_bootstrap);
    }

    // use the same structure as benchmark.data
    void load_data(vector<pair<array<double, D>, int>> data) {
        for(auto it = data.begin(); it != data.end(); ++it) {
            vector<double> record(D);
            for (uint i = 0; i < D; ++i) {
                record[i] = (*it).first[i];
            }
            pca.add_record(record);
        }
    }
    
    // below are wrappers of pca lib APIs 
    void solve() {
        pca.solve();
    }

    vector<double> get_principal(long index) {
        return pca.get_principal(index);
    }
    
    vector<double> get_eigenvector(long index) {
        return pca.get_eigenvector(index);
    }

    vector<double> get_eigenvalues() {
        return pca.get_eigenvalues();
    }

    double get_eigenvalue(long index) {
        return pca.get_eigenvalue(index);
    }
    
};