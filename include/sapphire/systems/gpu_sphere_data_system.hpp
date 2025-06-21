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
#include "sapphire/utility/data_buffers.hpp"

class GPUSphereDataSystem {
    public:
        GPUSphereDataSystem(bismuth::Registry& registry);

        void Update(bismuth::Registry& registry, DataBuffers& dataBuffer);
    private:
        void BindSpatial(DataBuffers& dataBuffer);
        void BindDensity(DataBuffers& dataBuffer);
        void BindForce(DataBuffers& dataBuffer);
        void BindPosToForce(DataBuffers& dataBuffer);

        void ComputeSpatialHash(const std::vector<uint32_t>& denseEntities, DataBuffers& dataBuffer);
        void ComputeDensity(const std::vector<uint32_t>& denseEntities, DataBuffers& dataBuffer);
        void ComputeForces(const std::vector<uint32_t>& denseEntities, DataBuffers& dataBuffer);
        void ComputePos(const std::vector<uint32_t>& denseEntities, DataBuffers& dataBuffer);

        void Render(const std::vector<uint32_t>& denseEntities, bismuth::Registry& registry, DataBuffers& dataBuffer);

    private:
        // Programs
        GLuint mDensityProgram;
        GLuint mForcesProgram;
        GLuint mPosProgram;
        GLuint mSpatialHashProgram;

        GLuint mRender;

        GLuint mDummyVAO;
};