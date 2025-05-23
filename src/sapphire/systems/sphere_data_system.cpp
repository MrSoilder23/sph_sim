#include "sapphire/systems/sphere_data_system.hpp"

std::vector<glm::vec4*> GetNeighbors(
    glm::vec4& position, bismuth::ComponentPool<SphereComponent>& spherePositions, float radius
) {
    std::vector<glm::vec4*> neighbors;
    float radiusSquared = radius * radius;

    auto endIt = spherePositions.ComponentEnd();
    for(auto it = spherePositions.ComponentBegin(); it != endIt; ++it) {
        const glm::vec3 point = position - it->positionAndRadius;
        if(glm::dot(point, point) <= radiusSquared) {
            neighbors.push_back(&it->positionAndRadius);
        }
    }
    return neighbors;
}
float ComputePressure(float& density, float restDensity = 1000.0f, float stiffness = 500.0f) {
    return stiffness * (density - restDensity);
}

void SphereDataSystem::Update(bismuth::Registry& registry) {
    auto particle = registry.GetView<SphereComponent, DensityComponent, PressureComponent, ForceComponent>();
    float radius = 2.0f;

    // To-Do change after bismuth iterator is improved
    // This for loop is 60% faster than provided from my class
    const auto viewEntities = particle.GetSmallestDense();
    auto& spherePool = registry.GetComponentPool<SphereComponent>();
    auto& densityPool = registry.GetComponentPool<DensityComponent>();
    auto& pressurePool = registry.GetComponentPool<PressureComponent>();
    auto& forcePool = registry.GetComponentPool<ForceComponent>();

    #pragma omp parallel for
    for(int i = 0; i < particle.SizeHint(); i++) {
        size_t entityID = (*viewEntities)[i];
        if(!spherePool.HasComponent(entityID) && !densityPool.HasComponent(entityID) && 
           !pressurePool.HasComponent(entityID) && !forcePool.HasComponent(entityID)) {
            continue;
        }

        // spherePool.GetComponent(entityID).positionAndRadius.y -= 0.1f;
        auto& point = spherePool.GetComponent(entityID).positionAndRadius;
        auto neighbors = GetNeighbors(point, spherePool, radius);

        float& density = densityPool.GetComponent(entityID).d;
        density = sapphire::ComputeDensity(point, neighbors, radius, 10.0f);
        pressurePool.GetComponent(entityID).p = ComputePressure(density);


    }
}