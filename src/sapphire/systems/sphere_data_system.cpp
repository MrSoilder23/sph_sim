#include "sapphire/systems/sphere_data_system.hpp"

void SphereDataSystem::Update(bismuth::Registry& registry) {
    using sapphire_config::SMOOTHING_LENGTH;
    float softening = 0.1f * SMOOTHING_LENGTH;

    auto& spherePool   = registry.GetComponentPool<SphereComponent>();
    auto& densityPool  = registry.GetComponentPool<DensityComponent>();
    auto& pressurePool = registry.GetComponentPool<PressureComponent>();
    auto& forcePool    = registry.GetComponentPool<ForceComponent>();
    auto& velocityPool = registry.GetComponentPool<VelocityComponent>();
    auto& massPool     = registry.GetComponentPool<MassComponent>();

    // SpatialHash
    auto& spatialPool = registry.GetComponentPool<SpatialHashComponent>();
    auto& posPool     = registry.GetComponentPool<PositionComponent>();

    auto& sphereIDs   = spherePool.GetDenseEntities();

    // Load whole array into cache for performance improvement
    // Dense component arrays
    auto& positionArray     = spherePool.GetDenseComponents();
    auto& densityArray      = densityPool.GetDenseComponents();
    auto& pressureArray     = pressurePool.GetDenseComponents();
    auto& velocityArray     = velocityPool.GetDenseComponents();
    auto& massArray         = massPool.GetDenseComponents();
    auto& spatialArray      = spatialPool.GetDenseComponents();
    auto& spatialPosArray   = posPool.GetDenseComponents();

    // Component locations
    auto& positionLocations = spherePool.GetComponentLocations();
    auto& densityLocations  = densityPool.GetComponentLocations();
    auto& pressureLocations = pressurePool.GetComponentLocations();
    auto& velocityLocations = velocityPool.GetComponentLocations();
    auto& massLocations     = massPool.GetComponentLocations();
    auto& spatialLocations  = spatialPool.GetComponentLocations();
    auto& spatialPosLoc     = posPool.GetComponentLocations();
    
    static std::vector<std::vector<size_t>> neighborsIDs;
    for(auto& neighbors : neighborsIDs) {
        neighbors.clear();
    }
    if(neighborsIDs.size() != sphereIDs.size()) {
        neighborsIDs.resize(sphereIDs.size());
    }

    #pragma omp parallel for
    for(int i = 0; i < sphereIDs.size(); i++) {
        size_t entityID = sphereIDs[i];
        
        GetNeighbors(
            entityID,
            SMOOTHING_LENGTH,
            neighborsIDs[i],
            sphereIDs.size(),

            positionArray.data(),
            spatialPosArray.data(),
            spatialArray.data(),
            positionLocations.data(),
            spatialPosLoc.data(),
            spatialLocations.data(),

            spatialPool.GetDenseEntities()
        );
    }

    #pragma omp parallel for
    for(int i = 0; i < sphereIDs.size(); i++) {
        size_t entityID = sphereIDs[i];

        float& pressure = pressurePool.GetComponent(entityID).p;
        float& density = densityPool.GetComponent(entityID).d;

        density = ComputeDensity(
            entityID,
            neighborsIDs[i],
            SMOOTHING_LENGTH,
            massPool.GetComponent(entityID).m,
            
            positionArray.data(),
            positionLocations.data()
        );
        pressure = ComputePressure(density);
    }

    #pragma omp parallel for
    for(int i = 0; i < sphereIDs.size(); i++) {
        size_t entityID = sphereIDs[i];

        glm::vec3& force = forcePool.GetComponent(entityID).f;
        force = ComputeForces(
            entityID,
            SMOOTHING_LENGTH,
            softening,
            neighborsIDs[i],

            positionArray.data(),
            velocityArray.data(),
            densityArray.data(),
            pressureArray.data(),
            massArray.data(),
            positionLocations.data(),
            densityLocations.data(),
            pressureLocations.data(),
            velocityLocations.data(),
            massLocations.data()
        );
    }
}

void SphereDataSystem::CheckNeighbor(int currentChunk, int& chunkNeighbor, int& neighbor) {
    if(neighbor < sapphire_config::SPATIAL_LENGTH && neighbor >= 0) {
        chunkNeighbor = currentChunk;
    } else if(neighbor < 0) {
        chunkNeighbor = currentChunk-1;
        neighbor = sapphire_config::SPATIAL_LENGTH-1;
    } else {
        chunkNeighbor = currentChunk+1;
        neighbor = 0;
    }
}

void SphereDataSystem::GetNeighbors(
    size_t               const& currentPointID,
    float                       radius,
    std::vector<size_t>&        neighbors, 
    size_t               const& maxParticles,

    SphereComponent      const* positionArray,
    PositionComponent    const* spatialPosArray,
    SpatialHashComponent const* spatialHashArray,
    uint32_t             const* positionLocations,
    uint32_t             const* spatialPositionLoc,
    uint32_t             const* spatialHashLocations,

    std::vector<uint32_t>  const& spatialDenseEntities
) {
    using sapphire_config::SPATIAL_LENGTH;
    using sapphire_config::SPATIAL_LENGTH_MAX;

    size_t count = 0;
    // neighbors.resize(maxParticles);
    size_t* tmp = (size_t*)alloca(maxParticles * sizeof(size_t));  

    float radiusSquaredMax = radius * radius;
    glm::vec3 currentPos = glm::vec3(positionArray[positionLocations[currentPointID]].positionAndRadius);

    // Translate global to local spatial space
    int chunkY = std::floor(currentPos.y / SPATIAL_LENGTH_MAX);
    int chunkX = std::floor(currentPos.x / SPATIAL_LENGTH_MAX);
    int chunkZ = std::floor(currentPos.z / SPATIAL_LENGTH_MAX);
    
    float localPosX = currentPos.x - SPATIAL_LENGTH_MAX*chunkX;
    float localPosY = currentPos.y - SPATIAL_LENGTH_MAX*chunkY;
    float localPosZ = currentPos.z - SPATIAL_LENGTH_MAX*chunkZ;
    
    int cubeX = std::floor(SPATIAL_LENGTH * (localPosX / SPATIAL_LENGTH_MAX));
    int cubeY = std::floor(SPATIAL_LENGTH * (localPosY / SPATIAL_LENGTH_MAX));
    int cubeZ = std::floor(SPATIAL_LENGTH * (localPosZ / SPATIAL_LENGTH_MAX));

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
                int flatIndex = SpatialHash::GetCoordinates(neighborX, neighborY, neighborZ);
                
                for(const auto& spatialID : spatialDenseEntities) {
                    const auto& spatialPos = spatialPosArray[spatialPositionLoc[spatialID]].position;

                    if(spatialPos != chunkKey) {
                        continue;
                    }
                    
                    const auto& spatial = spatialHashArray[spatialHashLocations[spatialID]];
                    const auto& entityArray = spatial.flatArrayIDs[flatIndex];
                    
                    for(const auto& ID : entityArray) {
                        const glm::vec3 point = currentPos - glm::vec3(positionArray[positionLocations[ID]].positionAndRadius);
                        const float radiusSquared = sapphire::Dot(point, point);
                        
                        if(radiusSquared <= radiusSquaredMax) {
                            tmp[count] = ID;
                            count++;
                        }
                    } 
                }
            }   
        }
    }

    neighbors.resize(count);
    if (count > 0) {
        std::memcpy(neighbors.data(),
                    tmp,
                    count * sizeof(size_t));
    }
}
float SphereDataSystem::ComputePressure(float& density) {
    return sapphire_config::STIFFNESS * (density - sapphire_config::REST_DENSITY);
}

float SphereDataSystem::ComputeDensity(
    size_t              const& currentPointID,
    std::vector<size_t> const& neighborIDs,
    float                      smoothingLength,
    float                      mass,
    
    SphereComponent     const* positionArray,
    uint32_t            const* positionLocations
) {
    float density = 0.0f;

    glm::vec3 point = glm::vec3(positionArray[positionLocations[currentPointID]].positionAndRadius);

    for(const auto& neighborID : neighborIDs) {
        glm::vec3 diff = point - glm::vec3(positionArray[positionLocations[neighborID]].positionAndRadius);
        float radius = sapphire::Length(diff, diff);

        density += mass * sapphire::CubicSplineKernel(radius, smoothingLength);
    }

    return std::max(density, 1e-5f);
}

glm::vec3 SphereDataSystem::ComputeForces(
    size_t            const&   currentPointID,
    float                      smoothingLength,
    float                      softening,
    std::vector<size_t> const& neighbors,

    SphereComponent   const*   positionArray,
    VelocityComponent const*   velocityArray,
    DensityComponent  const*   densityArray,
    PressureComponent const*   pressureArray,
    MassComponent     const*   massArray,
    uint32_t          const*   positionLocations,
    uint32_t          const*   densityLocations,
    uint32_t          const*   pressureLocations,
    uint32_t          const*   velocityLocations,
    uint32_t          const*   massLocations
) {
    glm::vec3 pressureForce(0.0f);
    glm::vec3 viscosityForce(0.0f);
    glm::vec3 gravityForce(0.0f);

    float softeningSquared = softening*softening;

    const glm::vec3 point = glm::vec3(positionArray[positionLocations[currentPointID]].positionAndRadius);

    const float& currentPointPressure     = pressureArray[pressureLocations[currentPointID]].p;
    const float& currentPointDensity      = densityArray[densityLocations[currentPointID]].d;
    const glm::vec3& currentPointVelocity = velocityArray[velocityLocations[currentPointID]].v;
    
    for(const auto& neighborID : neighbors) {
        glm::vec3 deltaPoint = point - glm::vec3(positionArray[positionLocations[neighborID]].positionAndRadius);
        float radiusSquared = sapphire::Dot(deltaPoint, deltaPoint);
        float radius = std::sqrt(radiusSquared);

        if(radius > 0.0f && radius < smoothingLength) {
            // Get neighbor components
            const float& neighborDensity      = densityArray[densityLocations[neighborID]].d;
            const float& neighborPressure     = pressureArray[pressureLocations[neighborID]].p;
            const float& neighborMass         = massArray[massLocations[neighborID]].m;
            const glm::vec3& neighborVelocity = velocityArray[velocityLocations[neighborID]].v;

            // Pressure
            float pressureTerm = (currentPointPressure / (currentPointDensity * currentPointDensity)) +
                (neighborPressure / (neighborDensity * neighborDensity));
            pressureForce += -pressureTerm * sapphire::CubicSplineGradient(deltaPoint, radius, smoothingLength);

            // Viscosity
            viscosityForce += (neighborDensity * (neighborVelocity - currentPointVelocity)) * sapphire::CubicSplineLaplacian(radius, smoothingLength);

            // Gravity
            float distSoft = radiusSquared + softeningSquared;
            float denominator = std::sqrt(distSoft*distSoft*distSoft);
            gravityForce += sapphire_config::G * neighborMass * deltaPoint / denominator;
        }
    }
    return pressureForce + viscosityForce + gravityForce;
}