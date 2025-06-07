#include "sapphire/systems/gpu_sphere_data_system.hpp"

GPUSphereDataSystem::GPUSphereDataSystem() {
    GLuint densityShader = shader::CompileShader(GL_COMPUTE_SHADER, "./shaders/compute/density.glsl");

    mDensityProgram = shader::LinkProgram(densityShader);
}

void GPUSphereDataSystem::Update(bismuth::Registry& registry) {
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

    
}

void GPUSphereDataSystem::CheckNeighbor(int currentChunk, int& chunkNeighbor, int& neighbor) {
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

void GPUSphereDataSystem::GetNeighbors(
    size_t               const& currentPointID,
    float                       radius,
    std::vector<size_t>&        neighbors, 
    size_t               const& maxParticles,

    SphereComponent      const* positionArray,
    PositionComponent    const* spatialPosArray,
    SpatialHashComponent const* spatialHashArray,
    size_t               const* positionLocations,
    size_t               const* spatialPositionLoc,
    size_t               const* spatialHashLocations,

    std::vector<size_t>  const& spatialDenseEntities
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