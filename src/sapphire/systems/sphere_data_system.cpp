#include "sapphire/systems/sphere_data_system.hpp"

void SphereDataSystem::Update(bismuth::Registry& registry) {
    auto particle = registry.GetView<SphereComponent, DensityComponent, PressureComponent, ForceComponent, VelocityComponent>();

    const static float smoothingLength = sapphire_config::SMOOTHING_LENGTH;
    float softening = 0.1f * smoothingLength;

    // To-Do change after bismuth iterator is improved
    // This for loop is 60% faster than provided from my class
    const auto viewEntities = particle.GetSmallestDense();
    auto& spherePool = registry.GetComponentPool<SphereComponent>();
    auto& densityPool = registry.GetComponentPool<DensityComponent>();
    auto& pressurePool = registry.GetComponentPool<PressureComponent>();
    auto& forcePool = registry.GetComponentPool<ForceComponent>();
    auto& velocityPool = registry.GetComponentPool<VelocityComponent>();
    auto& massPool = registry.GetComponentPool<MassComponent>();

    #pragma omp parallel for
    for(int i = 0; i < particle.SizeHint(); i++) {
        size_t entityID = (*viewEntities)[i];
        if(!spherePool.HasComponent(entityID) && !densityPool.HasComponent(entityID) && 
           !pressurePool.HasComponent(entityID) && !forcePool.HasComponent(entityID) &&
            !velocityPool.HasComponent(entityID)
        ) {
            continue;
        }

        auto& point = spherePool.GetComponent(entityID).positionAndRadius;
        auto neighborsIDs = GetNeighbors(entityID, point, spherePool, 2 * smoothingLength);
        
        std::vector<glm::vec4*> neighbors;
        neighbors.reserve(neighborsIDs.size());
        for(const auto& IDs : neighborsIDs) {
            neighbors.push_back(&spherePool.GetComponent(IDs).positionAndRadius);
        }

        float& density = densityPool.GetComponent(entityID).d;
        density = ComputeDensity(point, neighbors, smoothingLength, massPool.GetComponent(entityID).m);
        pressurePool.GetComponent(entityID).p = ComputePressure(density);
    }

    #pragma omp parallel for
    for(int i = 0; i < particle.SizeHint(); i++) {
        size_t entityID = (*viewEntities)[i];
        if(!spherePool.HasComponent(entityID) && !densityPool.HasComponent(entityID) && 
           !pressurePool.HasComponent(entityID) && !forcePool.HasComponent(entityID) &&
            !velocityPool.HasComponent(entityID)
        ) {
            continue;
        }

        auto& point = spherePool.GetComponent(entityID).positionAndRadius;
        auto neighborsIDs = GetNeighbors(entityID, point, spherePool, smoothingLength);

        glm::vec3& force = forcePool.GetComponent(entityID).f;
        force = ComputeForces(point, neighborsIDs, entityID,
            spherePool, pressurePool, densityPool, velocityPool,
            massPool, smoothingLength, softening);
    }
}

std::vector<size_t> SphereDataSystem::GetNeighbors(
    size_t& pointID, glm::vec4& position, bismuth::ComponentPool<SphereComponent>& spherePositions, float radius
) {
    std::vector<size_t> neighbors;
    float radiusSquared = radius * radius;
    glm::vec3 currentPos = glm::vec3(position);

    for(auto& entityID : spherePositions.GetDenseEntities()) {
        if(pointID == entityID) {
            continue;
        }
        const glm::vec3 point = currentPos - glm::vec3(spherePositions.GetComponent(entityID).positionAndRadius);

        if(glm::dot(point, point) <= radiusSquared) {
            neighbors.push_back(entityID);
        }
    }
    return neighbors;
}
float SphereDataSystem::ComputePressure(float& density) {
    return sapphire_config::STIFFNESS * (density - sapphire_config::REST_DENSITY);
}

float SphereDataSystem::ComputeDensity(const glm::vec4& point, const std::vector<glm::vec4*>& neighbors, float smoothingLength, float mass) {
    float density = 0.0f;

    for(const auto& neighbor : neighbors) {
        glm::vec3 diff = point - *neighbor;
        float radius = glm::length(diff);

        density += mass * sapphire::CubicSplineKernel(radius, smoothingLength);
    }

    return std::max(density, 1e-5f);
}

glm::vec3 SphereDataSystem::ComputeForces(
    glm::vec4& point, std::vector<size_t>& neighbors,
    size_t& currentPointID,
    bismuth::ComponentPool<SphereComponent>& spherePool, bismuth::ComponentPool<PressureComponent>& pressurePool,
    bismuth::ComponentPool<DensityComponent>& densityPool, bismuth::ComponentPool<VelocityComponent>& velocityPool,
    bismuth::ComponentPool<MassComponent>& massPool,
    float smoothingLength, float softening
) {
    glm::vec3 pressureForce(0.0f);
    glm::vec3 viscosityForce(0.0f);
    glm::vec3 gravityForce(0.0f);

    auto& currentPointPressure = pressurePool.GetComponent(currentPointID).p;
    auto& currentPointDensity = densityPool.GetComponent(currentPointID).d;
    auto& currentPointVelocity = velocityPool.GetComponent(currentPointID).v;

    currentPointDensity = std::max(currentPointDensity, 1e-5f);

    for(const auto& neighborID : neighbors) {
        glm::vec3 deltaPoint = glm::vec3(point) - glm::vec3(spherePool.GetComponent(neighborID).positionAndRadius);
        float radius = glm::length(deltaPoint);

        if(radius > 0.0f && radius < smoothingLength) {
            // Pressure
            glm::vec3 gradient = sapphire::CubicSplineGradient(deltaPoint, smoothingLength);

            auto& neighborDensity = densityPool.GetComponent(neighborID).d;
            neighborDensity = std::max(neighborDensity, 1e-5f);

            float pressureTerm = (currentPointPressure / (currentPointDensity * currentPointDensity)) +
                (pressurePool.GetComponent(neighborID).p / (neighborDensity * neighborDensity));
            pressureForce += -pressureTerm * gradient;

            // Viscosity
            glm::vec3 deltaVelocity = velocityPool.GetComponent(neighborID).v - currentPointVelocity;
            viscosityForce += (neighborDensity * deltaVelocity) * sapphire::CubicSplineLaplacian(radius, smoothingLength);

            // Gravity
            float distanceSquared = glm::dot(deltaPoint, deltaPoint);
            float denominator = std::pow(distanceSquared + softening*softening, 1.5f);
            gravityForce += sapphire_config::G * massPool.GetComponent(neighborID).m * deltaPoint / denominator;
        }
    }
    return pressureForce + viscosityForce + gravityForce;
}