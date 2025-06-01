#include "sapphire/systems/pos_to_spatial_system.hpp"

namespace std {
    template<>
    struct hash<glm::ivec3> {
        size_t operator()(const glm::ivec3& v) const noexcept {
            // Use large primes for good distribution
            const size_t h1 = hash<int>{}(v.x);
            const size_t h2 = hash<int>{}(v.y);
            const size_t h3 = hash<int>{}(v.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

void PosToSpatialSystem::Update(bismuth::Registry& registry) {
    constexpr float spatialSize = 32*sapphire_config::SMOOTHING_LENGTH;
    constexpr float invSpatialSize = 1.0f / spatialSize;
    constexpr int gridRes = 32;

    auto& particlePool = registry.GetComponentPool<SphereComponent>();
    auto& positionPool = registry.GetComponentPool<PositionComponent>();
    auto& spatialPool = registry.GetComponentPool<SpatialHashComponent>();

    const auto& particleIDs = particlePool.GetDenseEntities();
    const auto IDsToRemove = spatialPool.GetDenseEntities();

    std::unordered_map<glm::ivec3, size_t> chunkMap;
    chunkMap.reserve(IDsToRemove.size() * 2);

    for (const auto& entityID : spatialPool.GetDenseEntities()) {
        auto& spatial = spatialPool.GetComponent(entityID);
        const auto& pos = positionPool.GetComponent(entityID).position;
        glm::ivec3 chunkKey(static_cast<int>(pos.x), 
                           static_cast<int>(pos.y), 
                           static_cast<int>(pos.z));

        for (auto& bin : spatial.flatArrayIDs) {
            bin.clear();
        }

        chunkMap[chunkKey] = entityID;
    }


    for(const auto& particleID : particleIDs) {
        auto particlePos = glm::vec3(particlePool.GetComponent(particleID).positionAndRadius);

        glm::ivec3 chunkKey(
            static_cast<int>(std::floor(particlePos.x * invSpatialSize)),
            static_cast<int>(std::floor(particlePos.y * invSpatialSize)),
            static_cast<int>(std::floor(particlePos.z * invSpatialSize))
        );

        glm::vec3 localPos = particlePos - spatialSize * glm::vec3(chunkKey);
        glm::ivec3 cubeCoords(
            std::clamp(static_cast<int>(gridRes * localPos.x * invSpatialSize), 0, gridRes-1),
            std::clamp(static_cast<int>(gridRes * localPos.y * invSpatialSize), 0, gridRes-1),
            std::clamp(static_cast<int>(gridRes * localPos.z * invSpatialSize), 0, gridRes-1)
        );

        size_t binIndex = SpatialHash::GetCoordinates(cubeCoords.x, cubeCoords.y, cubeCoords.z);

        auto it = chunkMap.find(chunkKey);
        if (it != chunkMap.end()) {
            // Existing chunk
            auto& spatial = spatialPool.GetComponent(it->second);
            spatial.flatArrayIDs[binIndex].push_back(particleID);
        } else {
            // Create new chunk
            size_t newEntity = registry.CreateEntity();
            registry.EmplaceComponent<PositionComponent>(newEntity, glm::vec3(chunkKey));
            registry.EmplaceComponent<SpatialHashComponent>(newEntity);

            auto& spatial = spatialPool.GetComponent(newEntity);
            spatial.flatArrayIDs[binIndex].push_back(particleID);
            chunkMap[chunkKey] = newEntity;
        }
    }

    auto& spatialEntities = spatialPool.GetDenseEntities();
    for (auto it = spatialEntities.begin(); it != spatialEntities.end(); ) {
        auto& spatial = spatialPool.GetComponent(*it);
        bool isEmpty = true;
        for (const auto& bin : spatial.flatArrayIDs) {
            if (!bin.empty()) {
                isEmpty = false;
                break;
            }
        }
        
        if (isEmpty) {
            registry.RemoveEntity(*it);
        } else {
            ++it;
        }
    }

}