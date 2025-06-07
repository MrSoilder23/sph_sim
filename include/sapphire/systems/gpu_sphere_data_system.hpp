#pragma once
// Own libraries
#include "bismuth/registry.hpp"
#include "quartz/graphics/shader.hpp"
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

class GPUSphereDataSystem {
    public:
        GPUSphereDataSystem();

        void Update(bismuth::Registry& registry);
    private:
        void CheckNeighbor(int currentChunk, int& chunkNeighbor, int& neighbor);

        void GetNeighbors(
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
        );

    private:
        GLuint mDensityProgram;
};