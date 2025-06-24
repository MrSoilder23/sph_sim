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
    void Init(bismuth::Registry& registry);
    void SyncData(bismuth::Registry& registry);
    void UpdateBuffers(bismuth::Registry& registry);

    // Helpers
    template<typename T>
    void GenerateBuffers(GLuint& buffer, const std::vector<T>& data) {
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), GL_DYNAMIC_COPY);
    }

    template<typename T>
    void FillBuffer(GLuint& buffer, const std::vector<T>& data) {
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