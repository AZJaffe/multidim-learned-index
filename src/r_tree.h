#pragma once

#include <iostream>
#include <random>
#include <chrono>
#include <spatialindex/SpatialIndex.h>
#include <spatialindex/capi/CountVisitor.h>

// Wrapper around spatialindex Rtree
template <uint D, typename V>
// Doesnt use V
class RTree {
    typedef pair<array<double, D>, V> datum;
    SpatialIndex::ISpatialIndex * t;
public:
    RTree(vector<datum> & data) {
        SpatialIndex::IStorageManager * memStore = SpatialIndex::StorageManager::createNewMemoryStorageManager();
        SpatialIndex::id_type id = 0;
        // TODO - use createAndBulkLoadNewRTree instead.
        t = SpatialIndex::RTree::createNewRTree(
            *memStore,
            0.5, // fill factor
            1000, // index capacity
            1000, // leaf capacity,
            D, // dimension
            SpatialIndex::RTree::RV_RSTAR, // type
            id // id (not sure why this is needed)
        ); // TODO - tune parameters?
        for(auto it = data.begin(); it != data.end(); it++) {
            cout << "Adding data point: " << it - data.begin() << endl;
            SpatialIndex::id_type id = 0;
            t->insertData(
                0, nullptr, // Don't insert data, just the point
                SpatialIndex::Point(it->first.data(), D),
                id
            );
        }
    };

    size_t rangeQuery(array<double, D> min, array<double, D> max) {
        auto r = SpatialIndex::Region(min.data(), max.data(), D);
        auto v = CountVisitor();
        t->intersectsWithQuery(r, v);
        return v.GetResultCount();
    }
};
