#pragma once
// C++ standard libraries
#include <unordered_map>

// Third_party libraries
#include <glm/glm.hpp>

// Own libraries
#include "bismuth/registry.hpp"
#include "sapphire/utility/utility.hpp"
#include "sapphire/utility/config.hpp"
#include "sapphire/components/density_component.hpp"
#include "sapphire/components/force_component.hpp"
#include "sapphire/components/pressure_component.hpp"
#include "sapphire/components/velocity_component.hpp"
#include "sapphire/components/mass_component.hpp"
#include "quartz/core/components/sphere_component.hpp"

#include "sapphire/components/position_component.hpp"
#include "sapphire/components/spatial_hash_component.hpp"

class SphereDataSystem {
    public:
        void Update(bismuth::Registry& registry);
    private:
        void CheckNeighbor(int currentChunk, int& chunkNeighbor, int& neighbor);

        void GetNeighbors(std::vector<size_t>& neighbors, size_t& pointID, size_t maxParticles,
            bismuth::ComponentPool<SphereComponent>& spherePositions,
            bismuth::ComponentPool<SpatialHashComponent>& spatialHash, bismuth::ComponentPool<PositionComponent>& posPool,
            float radius);

        float ComputePressure(float& density);
        float ComputeDensity(const glm::vec4& point, const std::vector<size_t>& neighborIDs, 
            bismuth::ComponentPool<SphereComponent>& spherePool, float smoothingLength, float mass);

        glm::vec3 ComputeForces(
            glm::vec4& point, std::vector<size_t>& neighbors,
            size_t& currentPointID,
            bismuth::ComponentPool<SphereComponent>& spherePool, bismuth::ComponentPool<PressureComponent>& pressurePool,
            bismuth::ComponentPool<DensityComponent>& densityPool, bismuth::ComponentPool<VelocityComponent>& velocityPool,
            bismuth::ComponentPool<MassComponent>& massPool,
            float smoothingLength, float softening
        );
};