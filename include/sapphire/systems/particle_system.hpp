#pragma once
// Third_party libraries
#include <glm/glm.hpp>

// Own libraries
#include "bismuth/registry.hpp"

#include "quartz/core/components/instance_component.hpp"
#include "quartz/core/components/sphere_component.hpp"
#include "sapphire/components/density_component.hpp"
#include "sapphire/components/force_component.hpp"
#include "sapphire/components/mass_component.hpp"
#include "sapphire/components/pressure_component.hpp"
#include "sapphire/components/velocity_component.hpp"
#include "sapphire/components/energy_component.hpp"

class ParticleSystem {
    public:
        ParticleSystem(bismuth::Registry& registry);
        
        void CreateParticle(float x, float y, float z, float mass, glm::vec4 velocity);
    private:
        bismuth::Registry& mRegistry;
};