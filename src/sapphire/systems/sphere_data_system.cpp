#include "sapphire/systems/sphere_data_system.hpp"

void SphereDataSystem::Update(bismuth::Registry& registry) {
    auto particle = registry.GetView<SphereComponent, DensityComponent, PressureComponent, ForceComponent, VelocityComponent>();

    const static float smoothingLength = sapphire_config::SMOOTHING_LENGTH;
    float softening = 0.1f * smoothingLength;

    // To-Do change after bismuth iterator is improved
    // This for loop is 60% faster than provided from my class
    const auto& viewEntities = particle.GetSmallestDense();
    auto& spherePool = registry.GetComponentPool<SphereComponent>();
    auto& densityPool = registry.GetComponentPool<DensityComponent>();
    auto& pressurePool = registry.GetComponentPool<PressureComponent>();
    auto& forcePool = registry.GetComponentPool<ForceComponent>();
    auto& velocityPool = registry.GetComponentPool<VelocityComponent>();
    auto& massPool = registry.GetComponentPool<MassComponent>();

    // SpatialHash
    auto& spatialPool = registry.GetComponentPool<SpatialHashComponent>();
    auto& posPool = registry.GetComponentPool<PositionComponent>();

    auto& sphereIDs = spherePool.GetDenseEntities();
    
    std::vector<std::vector<size_t>> neighborsIDs;
    neighborsIDs.resize(sphereIDs.size());

    #pragma omp parallel for
    for(int i = 0; i < sphereIDs.size(); i++) {
        size_t entityID = sphereIDs[i];
        
        GetNeighbors(neighborsIDs[i], entityID, sphereIDs.size(), spherePool, spatialPool, posPool, smoothingLength);
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

        float& density = densityPool.GetComponent(entityID).d;
        density = ComputeDensity(point, neighborsIDs[i], spherePool, smoothingLength, massPool.GetComponent(entityID).m);
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

        glm::vec3& force = forcePool.GetComponent(entityID).f;
        force = ComputeForces(point, neighborsIDs[i], entityID,
            spherePool, pressurePool, densityPool, velocityPool,
            massPool, smoothingLength, softening);
    }
}

void SphereDataSystem::CheckNeighbor(int currentChunk, int& chunkNeighbor, int& neighbor) {
    if(neighbor < 32 && neighbor >= 0) {
        chunkNeighbor = currentChunk;
    } else if(neighbor < 0) {
        chunkNeighbor = currentChunk-1;
        neighbor = 31;
    } else {
        chunkNeighbor = currentChunk+1;
        neighbor = 0;
    }
}

void SphereDataSystem::GetNeighbors(
    std::vector<size_t>& neighbors, size_t& pointID, size_t maxParticles, bismuth::ComponentPool<SphereComponent>& spherePositions, 
    bismuth::ComponentPool<SpatialHashComponent>& spatialHash, bismuth::ComponentPool<PositionComponent>& posPool,
    float radius
) {
    constexpr float spatialSize = 32*sapphire_config::SMOOTHING_LENGTH;
    constexpr int gridRes = 32;

    size_t count = 0;
    neighbors.resize(maxParticles);

    float radiusSquared = radius * radius;
    glm::vec3 currentPos = glm::vec3(spherePositions.GetComponent(pointID).positionAndRadius);

    // Translate global to local spatial space
    int chunkY = std::floor(currentPos.y / spatialSize);
    int chunkX = std::floor(currentPos.x / spatialSize);
    int chunkZ = std::floor(currentPos.z / spatialSize);
    
    float localPosX = currentPos.x - spatialSize*chunkX;
    float localPosY = currentPos.y - spatialSize*chunkY;
    float localPosZ = currentPos.z - spatialSize*chunkZ;
    
    int cubeX = std::floor(gridRes * (localPosX / spatialSize));
    int cubeY = std::floor(gridRes * (localPosY / spatialSize));
    int cubeZ = std::floor(gridRes * (localPosZ / spatialSize));

    for(int localX = -1; localX < 2; localX++) {
        for(int localY = -1; localY < 2; localY++) {
            for(int localZ = -1; localZ < 2; localZ++) {
                int neighborX = cubeX + localX;
                int neighborY = cubeY + localY;
                int neighborZ = cubeZ + localZ;

                int chunkNeighborX;
                int chunkNeighborY;
                int chunkNeighborZ;

                CheckNeighbor(chunkX, chunkNeighborX, neighborX);
                CheckNeighbor(chunkY, chunkNeighborY, neighborY);
                CheckNeighbor(chunkZ, chunkNeighborZ, neighborZ);

                glm::vec3 chunkKey(chunkNeighborX, chunkNeighborY, chunkNeighborZ);
                
                for(const auto& spatialID : spatialHash.GetDenseEntities()) {
                    const auto& spatialPos = posPool.GetComponent(spatialID).position;

                    if(spatialPos != chunkKey) {
                        continue;
                    }
                    
                    int flatIndex = SpatialHash::GetCoordinates(neighborX, neighborY, neighborZ);
                    
                    const auto& spatial = spatialHash.GetComponent(spatialID);
                    const auto& entityArray = spatial.flatArrayIDs[flatIndex];
                    
                    for(const auto& ID : entityArray) {
                        const glm::vec3 point = currentPos - glm::vec3(spherePositions.GetComponent(ID).positionAndRadius);
                        
                        if(glm::dot(point, point) <= radiusSquared) {
                            neighbors[count] = ID;
                            count++;
                        }
                    } 
                }
            }   
        }
    }
    neighbors.resize(count);
}
float SphereDataSystem::ComputePressure(float& density) {
    return sapphire_config::STIFFNESS * (density - sapphire_config::REST_DENSITY);
}

float SphereDataSystem::ComputeDensity(const glm::vec4& point, const std::vector<size_t>& neighborIDs,
    bismuth::ComponentPool<SphereComponent>& spherePool, float smoothingLength, float mass
) {
    float density = 0.0f;

    for(const auto& neighborID : neighborIDs) {
        glm::vec3 diff = point - spherePool.GetComponent(neighborID).positionAndRadius;
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