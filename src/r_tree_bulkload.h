#include <spatialindex/SpatialIndex.h>

using namespace SpatialIndex;

class PointDataStream : public IDataStream {
public:
    std::ifstream m_fin;
    RTree::Data* m_pNext;
    PointDataStream(std::string inputFile) : m_pNext(nullptr) {
        m_fin.open(inputFile.c_str());

        if (!m_fin) {
            throw Tools::IllegalArgumentException("Input file not found");
        }

        readNextEntry();
    }

    ~PointDataStream() override {
        if (m_pNext != nullptr) delete m_pNext;
    }

    IData* getNext() override {
        if (m_pNext == nullptr) return nullptr;

        RTree::Data* ret = m_pNext;
        m_pNext = nullptr;
        readNextEntry();
        return ret;
    }

    bool hasNext() override {
        return (m_pNext != nullptr);
    }

    uint32_t size() override {
        throw Tools::NotSupportedException("Operation not supported.");
    }

    void rewind() override {
        if (m_pNext != nullptr) {
            delete m_pNext;
            m_pNext = nullptr;
        }

        m_fin.seekg(0, std::ios::beg);
        readNextEntry();
    }

    void readNextEntry() {
        id_type id = 0; // id set manually
        double x, y;
        char dummy;
        
        m_fin >> x >> dummy >> y;

        double low[2] = {x, y};
        double high[2] = {x, y};
        if (m_fin.good()) {
            Region r(low, high, 2);
            m_pNext = new RTree::Data(sizeof(double), reinterpret_cast<uint8_t*>(low), r, id);
        }
    }
};