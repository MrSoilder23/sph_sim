#pragma once
// Own libraries
#include "bismuth/registry.hpp"
#include "sapphire/components/force_component.hpp"
#include "sapphire/components/velocity_component.hpp"
#include "sapphire/components/mass_component.hpp"
#include "quartz/core/components/sphere_component.hpp"

class ForceToPosSystem {
    public:
        void Update(bismuth::Registry& registry, float deltaTime);
};