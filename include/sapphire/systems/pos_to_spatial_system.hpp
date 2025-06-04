#pragma once
// C++ standard libraries
#include <unordered_map>

// Own libraries
#include "bismuth/registry.hpp"
#include "quartz/core/components/sphere_component.hpp"
#include "sapphire/components/position_component.hpp"
#include "sapphire/components/spatial_hash_component.hpp"
#include "sapphire/utility/config.hpp"

class PosToSpatialSystem {
    public:
        void Update(bismuth::Registry& registry);
};