#include "sapphire/systems/particle_system.hpp"

ParticleSystem::ParticleSystem(bismuth::Registry& registry) :mRegistry(registry) {
}

void ParticleSystem::CreateParticle(float x, float y, float z, float mass, glm::vec4 velocity) {
    size_t sphereEntity = mRegistry.CreateEntity();
            
    mRegistry.EmplaceComponent<InstanceComponent>(sphereEntity);
    mRegistry.EmplaceComponent<SphereComponent>(sphereEntity, glm::vec4(x, y, z, 1));

    mRegistry.EmplaceComponent<DensityComponent>(sphereEntity,  0.0f);
    mRegistry.EmplaceComponent<PressureComponent>(sphereEntity, 0.0f);
    mRegistry.EmplaceComponent<MassComponent>(sphereEntity,     mass);
    mRegistry.EmplaceComponent<ForceComponent>(sphereEntity,    glm::vec4(0.0f));
    mRegistry.EmplaceComponent<VelocityComponent>(sphereEntity, velocity);
}