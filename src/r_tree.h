#pragma once

#include <iostream>
#include <random>
#include <chrono>
#include <spatialindex/SpatialIndex.h>
#include <spatialindex/capi/CountVisitor.h>
#include "r_tree_bulkload.h"

// Wrapper around spatialindex Rtree
template <uint D, typename V>
// Doesnt use V
class CustomRTree {
    typedef pair<array<double, D>, V> datum;
    SpatialIndex::ISpatialIndex * t;
public:
    CustomRTree(vector<datum> & data) {
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
            SpatialIndex::id_type id = 0;
            t->insertData(
                0, nullptr, // Don't insert data, just the point
                SpatialIndex::Point(it->first.data(), D),
                id
            );
        }
    };

    // bulkload from file
    CustomRTree(std::string fileName) {
        SpatialIndex::IStorageManager * memStorage = SpatialIndex::StorageManager::createNewMemoryStorageManager();
        SpatialIndex::StorageManager::IBuffer * fileInMem = SpatialIndex::StorageManager::createNewRandomEvictionsBuffer(*memStorage, 1000, false);
        double fillFactor = 0.5;
        size_t indexCapacity = 1000;
        size_t leafCapacity = 1000;
        size_t dimension = D;
        id_type indexIndentifier;
        PointDataStream dstream(fileName);
        t = SpatialIndex::RTree::createAndBulkLoadNewRTree(SpatialIndex::RTree::BLM_STR, dstream, *fileInMem,
        fillFactor, indexCapacity, leafCapacity, dimension, SpatialIndex::RTree::RV_RSTAR, indexIndentifier);
    };

    size_t memorySize() {
        SpatialIndex::IStatistics *s;
        t->getStatistics(&s); 
        return s->getNumberOfNodes() * sizeof(SpatialIndex::INode) + s->getNumberOfData() * sizeof(SpatialIndex::IData);
    }

    size_t rangeQuery(array<double, D> min, array<double, D> max) {
        auto r = SpatialIndex::Region(min.data(), max.data(), D);
        auto v = CountVisitor();
        t->intersectsWithQuery(r, v);
        return v.GetResultCount();
    }
};
