#pragma once
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

        void GetNeighbors(    
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
        );

        float ComputePressure(float& density);
        float ComputeDensity(
            size_t              const& currentPointID,
            std::vector<size_t> const& neighborIDs,
            float                      smoothingLength,
            float                      mass,
            
            SphereComponent     const* positionArray,
            uint32_t            const* positionLocations    
        );

        glm::vec3 ComputeForces(
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
        );
};