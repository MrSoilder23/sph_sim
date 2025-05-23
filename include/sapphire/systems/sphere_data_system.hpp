#pragma once
// Third_party libraries
#include <glm/glm.hpp>

// Own libraries
#include "bismuth/registry.hpp"
#include "sapphire/systems/utility.hpp"
#include "sapphire/components/density_component.hpp"
#include "sapphire/components/force_component.hpp"
#include "sapphire/components/pressure_component.hpp"
#include "quartz/core/components/sphere_component.hpp"

class SphereDataSystem {
    public:
        void Update(bismuth::Registry& registry);
};