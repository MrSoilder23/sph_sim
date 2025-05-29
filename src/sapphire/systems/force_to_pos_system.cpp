#include "sapphire/systems/force_to_pos_system.hpp"

void ForceToPosSystem::Update(bismuth::Registry& registry, float deltaTime) {
    auto particle = registry.GetView<SphereComponent, MassComponent, ForceComponent, VelocityComponent>();

    // To-Do change after bismuth iterator is improved
    // This for loop is 60% faster than provided from my class
    const auto viewEntities = particle.GetSmallestDense();
    auto& spherePool = registry.GetComponentPool<SphereComponent>();
    auto& forcePool = registry.GetComponentPool<ForceComponent>();
    auto& velocityPool = registry.GetComponentPool<VelocityComponent>();
    auto& massPool = registry.GetComponentPool<MassComponent>();

    #pragma omp parallel for
    for(int i = 0; i < particle.SizeHint(); i++) {
        size_t entityID = (*viewEntities)[i];
        if(!spherePool.HasComponent(entityID) && !velocityPool.HasComponent(entityID) && 
           !forcePool.HasComponent(entityID)) {
            continue;
        }

        glm::vec3& velocity = velocityPool.GetComponent(entityID).v;
        auto& mass = massPool.GetComponent(entityID).m;
        auto& force = forcePool.GetComponent(entityID).f;

        glm::vec3 acceleration = force / mass;
        velocity += acceleration * 0.01f;

        spherePool.GetComponent(entityID).positionAndRadius += glm::vec4(velocity, 0.0f) * 0.01f;
    }
}