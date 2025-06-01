#pragma once
// C++ standard libraries
#include <vector>

struct SpatialHashComponent {
    std::vector<std::vector<size_t>> flatArrayIDs;

    SpatialHashComponent() : flatArrayIDs(32768) {}
};

namespace SpatialHash {
    inline size_t GetCoordinates(const int& x,const int& y,const int& z) {
        assert(x < 32 && y < 32 && z < 32 && 
               x >= 0 && y >= 0 && z >= 0);

        return x + y * 32 + z * 32*32;
    }
}