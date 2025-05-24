#include "sapphire/systems/sphere_data_system.hpp"

std::vector<const size_t*> GetNeighbors(
    glm::vec4& position, bismuth::ComponentPool<SphereComponent>& spherePositions, float radius
) {
    std::vector<const size_t*> neighbors;
    float radiusSquared = radius * radius;

    for(auto& entityID : spherePositions.GetDenseEntities()) {
        const glm::vec3 point = position - spherePositions.GetComponent(entityID).positionAndRadius;
        if(glm::dot(point, point) <= radiusSquared) {
            neighbors.push_back(&entityID);
        }
    }
    return neighbors;
}
float ComputePressure(float& density, float restDensity = 1000.0f, float stiffness = 500.0f) {
    return stiffness * (density - restDensity);
}

glm::vec3 ComputeForces(
    glm::vec4& point, std::vector<const size_t*>& neighbors,
    size_t& currentPointID,
    bismuth::ComponentPool<SphereComponent> spherePool, bismuth::ComponentPool<PressureComponent> pressurePool,
    bismuth::ComponentPool<DensityComponent> densityPool, bismuth::ComponentPool<VelocityComponnet> velocityPool, 
    float smoothingLength
) {
    glm::vec3 pressureForce(0.0f);
    glm::vec3 viscosityForce(0.0f);

    auto& currentPointPressure = pressurePool.GetComponent(currentPointID).p;
    auto& currentPointDensity = densityPool.GetComponent(currentPointID).d;
    auto& currentPointVelocity = velocityPool.GetComponent(currentPointID).v;

    for(const auto& neighborID : neighbors) {
        glm::vec3 deltaPoint = point - spherePool.GetComponent(*neighborID).positionAndRadius;
        float radius = glm::length(deltaPoint);

        if(radius > 0.0f && radius < smoothingLength) {
            glm::vec3 gradient = sapphire::CubicSplineGradient(deltaPoint, smoothingLength);

            auto& neighborDensity = densityPool.GetComponent(*neighborID).d;

            float pressureTerm = (currentPointPressure / (currentPointDensity * currentPointDensity)) +
                (pressurePool.GetComponent(*neighborID).p / (neighborDensity * neighborDensity));
            pressureForce += - pressureTerm * gradient;

            glm::vec3 deltaVelocity = velocityPool.GetComponent(*neighborID).v - currentPointVelocity;
            viscosityForce += (neighborDensity * deltaVelocity) * sapphire::QuinticKernel(radius, smoothingLength);
        }
    }
    return pressureForce + viscosityForce;
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
    auto& velocityPool = registry.GetComponentPool<VelocityComponnet>();

    #pragma omp parallel for
    for(int i = 0; i < particle.SizeHint(); i++) {
        size_t entityID = (*viewEntities)[i];
        if(!spherePool.HasComponent(entityID) && !densityPool.HasComponent(entityID) && 
           !pressurePool.HasComponent(entityID) && !forcePool.HasComponent(entityID)) {
            continue;
        }

        auto& point = spherePool.GetComponent(entityID).positionAndRadius;
        auto neighborsIDs = GetNeighbors(point, spherePool, radius);
        
        std::vector<glm::vec4*> neighbors;
        neighbors.reserve(neighborsIDs.size());
        for(const auto& IDs : neighborsIDs) {
            neighbors.push_back(&spherePool.GetComponent(*IDs).positionAndRadius);
        }

        float& density = densityPool.GetComponent(entityID).d;
        density = sapphire::ComputeDensity(point, neighbors, radius, 10.0f);
        pressurePool.GetComponent(entityID).p = ComputePressure(density);

        glm::vec3& force = forcePool.GetComponent(entityID).f;
        force = ComputeForces(point, neighborsIDs, entityID,
            spherePool, pressurePool, densityPool, velocityPool, 
            radius);
    }
}