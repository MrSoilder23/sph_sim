#pragma once
// C++ standard libraries
#include <vector>

// Own libraries
#include "sapphire/utility/config.hpp"

struct SpatialHashComponent {
    std::vector<std::vector<size_t>> flatArrayIDs;

    SpatialHashComponent() : flatArrayIDs(sapphire_config::SPATIAL_SIZE) {}
};

namespace SpatialHash {
    inline size_t GetCoordinates(const int& x,const int& y,const int& z) {
        using sapphire_config::SPATIAL_LENGTH;
        
        assert(x < SPATIAL_LENGTH && y < SPATIAL_LENGTH && z < SPATIAL_LENGTH && 
               x >= 0 && y >= 0 && z >= 0);

        return x + y * SPATIAL_LENGTH + z * SPATIAL_LENGTH*SPATIAL_LENGTH;
    }
}