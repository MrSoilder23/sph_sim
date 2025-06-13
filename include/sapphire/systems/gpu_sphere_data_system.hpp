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
        GPUSphereDataSystem(bismuth::Registry& registry);

        void Update(bismuth::Registry& registry);
    private:
        void BindDensity();
        void BindForce();
        void BindPosToForce();

        void ComputeDensity(const std::vector<uint32_t>& denseEntities);
        void ComputeForces(const std::vector<uint32_t>& denseEntities);
        void ComputePos(const std::vector<uint32_t>& denseEntities);

        void Render(const std::vector<uint32_t>& denseEntities, bismuth::Registry& registry);

    private:
        GLuint mDensityProgram;
        GLuint mForcesProgram;
        GLuint mPosProgram;

        GLuint mRender;

        GLuint mSphereData;
        GLuint mMassData;
        GLuint mDensityData;
        GLuint mPressureData;
        GLuint mForceData;
        GLuint mVelocityData;

        GLuint mSphereLocData;
        GLuint mMassLocData;
        GLuint mDensityLocData;
        GLuint mPressureLocData;
        GLuint mForceLocData;
        GLuint mVelocityLocData;

        GLuint mDenseIDs;

        GLuint mDummyVAO;
};