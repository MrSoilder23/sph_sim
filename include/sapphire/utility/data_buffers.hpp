#pragma once
// Third party libraries
#include <glad/glad.h>

// Own libraries
#include "bismuth/registry.hpp"
#include "sapphire/utility/config.hpp"
#include "sapphire/components/density_component.hpp"
#include "sapphire/components/force_component.hpp"
#include "sapphire/components/pressure_component.hpp"
#include "sapphire/components/velocity_component.hpp"
#include "sapphire/components/mass_component.hpp"
#include "quartz/core/components/sphere_component.hpp"

struct DataBuffers {
    void Init(bismuth::Registry& registry) {
        auto& spherePool        = registry.GetComponentPool<SphereComponent>();
        auto& densityPool       = registry.GetComponentPool<DensityComponent>();
        auto& pressurePool      = registry.GetComponentPool<PressureComponent>();
        auto& forcePool         = registry.GetComponentPool<ForceComponent>();
        auto& velocityPool      = registry.GetComponentPool<VelocityComponent>();
        auto& massPool          = registry.GetComponentPool<MassComponent>();

        // Load whole array into cache for performance improvement
        // Dense component arrays
        auto& positionArray     = spherePool.GetDenseComponents();
        auto& densityArray      = densityPool.GetDenseComponents();
        auto& pressureArray     = pressurePool.GetDenseComponents();
        auto& velocityArray     = velocityPool.GetDenseComponents();
        auto& massArray         = massPool.GetDenseComponents();
        auto& forceArray        = forcePool.GetDenseComponents();

        // Component locations
        auto& positionLocations = spherePool.GetComponentLocations();
        auto& densityLocations  = densityPool.GetComponentLocations();
        auto& pressureLocations = pressurePool.GetComponentLocations();
        auto& velocityLocations = velocityPool.GetComponentLocations();
        auto& massLocations     = massPool.GetComponentLocations();
        auto& forceLocations    = forcePool.GetComponentLocations();

        auto& denseEntities     = spherePool.GetDenseEntities();
        
        
        constexpr uint32_t RESET_VALUE = 0xFFFFFFFF;

        // Data Buffers
        GenerateBuffers(mSphereData,      positionArray);
        GenerateBuffers(mMassData,        massArray);
        GenerateBuffers(mDensityData,     densityArray);
        GenerateBuffers(mPressureData,    pressureArray);
        GenerateBuffers(mVelocityData,    velocityArray);
        GenerateBuffers(mForceData,       forceArray);

        // Location Buffers
        GenerateBuffers(mSphereLocData,   positionLocations);
        GenerateBuffers(mMassLocData,     massLocations);
        GenerateBuffers(mDensityLocData,  densityLocations);
        GenerateBuffers(mPressureLocData, pressureLocations);
        GenerateBuffers(mVelocityLocData, velocityLocations);
        GenerateBuffers(mForceLocData,    forceLocations);

        GenerateBuffers(mDenseIDs,        denseEntities);

        // SpatialHash
        glGenBuffers(1, &mHashTable);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mHashTable);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sapphire_config::HASH_SIZE * sizeof(uint32_t), nullptr, GL_DYNAMIC_COPY);
        glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &RESET_VALUE);

        glGenBuffers(1, &mNextPointers);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mNextPointers);
        glBufferData(GL_SHADER_STORAGE_BUFFER, denseEntities.size() * sizeof(uint32_t), nullptr, GL_DYNAMIC_COPY);
        glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &RESET_VALUE);

        glGenBuffers(1, &mBucketKeys);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBucketKeys);
        glBufferData(GL_SHADER_STORAGE_BUFFER, denseEntities.size() * sizeof(uint32_t), nullptr, GL_DYNAMIC_COPY);
        glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &RESET_VALUE);
    }

    template<typename T>
    void GenerateBuffers(GLuint& buffer, const std::vector<T>& data) {
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), GL_DYNAMIC_COPY);
    }

    // Data SSBO
    GLuint mSphereData;
    GLuint mMassData;
    GLuint mDensityData;
    GLuint mPressureData;
    GLuint mForceData;
    GLuint mVelocityData;

    // Locations SSBO
    GLuint mSphereLocData;
    GLuint mMassLocData;
    GLuint mDensityLocData;
    GLuint mPressureLocData;
    GLuint mForceLocData;
    GLuint mVelocityLocData;

    GLuint mDenseIDs;

    // SparseHash SSBO
    GLuint mHashTable;
    GLuint mNextPointers;
    GLuint mBucketKeys;
};