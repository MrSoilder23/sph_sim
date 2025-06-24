#include "sapphire/utility/data_buffers.hpp"

void DataBuffers::Init(bismuth::Registry& registry) {
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

void DataBuffers::SyncData(bismuth::Registry& registry) {
    auto& particlePool     = registry.GetComponentPool<SphereComponent>();
    auto& densityPool      = registry.GetComponentPool<DensityComponent>();
    auto& pressurePool     = registry.GetComponentPool<PressureComponent>();
    auto& forcePool        = registry.GetComponentPool<ForceComponent>();
    auto& velocityPool     = registry.GetComponentPool<VelocityComponent>();

    auto& denseParticleIDs = particlePool.GetDenseEntities();

    const size_t sphereSize   = denseParticleIDs.size() * sizeof(SphereComponent);
    const size_t densitySize  = denseParticleIDs.size() * sizeof(DensityComponent);
    const size_t pressureSize = denseParticleIDs.size() * sizeof(PressureComponent);
    const size_t forceSize    = denseParticleIDs.size() * sizeof(ForceComponent);
    const size_t velocitySize = denseParticleIDs.size() * sizeof(VelocityComponent);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSphereData);
    glm::vec4* posPtr = static_cast<glm::vec4*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sphereSize, GL_MAP_READ_BIT));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mDensityData);
    float* densityPtr = static_cast<float*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, densitySize, GL_MAP_READ_BIT));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mPressureData);
    float* pressurePtr = static_cast<float*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, pressureSize, GL_MAP_READ_BIT));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mForceData);
    glm::vec4* forcePtr = static_cast<glm::vec4*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, forceSize, GL_MAP_READ_BIT));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mVelocityData);
    glm::vec4* velocityPtr = static_cast<glm::vec4*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, velocitySize, GL_MAP_READ_BIT));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    auto posBegin = particlePool.ComponentBegin();
    auto posEnd   = particlePool.ComponentEnd();
    for(auto it = posBegin; it != posEnd; ++it) {
        it->positionAndRadius = *posPtr++;
    }

    auto densityBegin = densityPool.ComponentBegin();
    auto densityEnd   = densityPool.ComponentEnd();
    for(auto it = densityBegin; it != densityEnd; ++it) {
        it->d = *densityPtr++;
    }

    auto pressureBegin = pressurePool.ComponentBegin();
    auto pressureEnd   = pressurePool.ComponentEnd();
    for(auto it = pressureBegin; it != pressureEnd; ++it) {
        it->p = *pressurePtr++;
    }

    auto forceBegin = forcePool.ComponentBegin();
    auto forceEnd   = forcePool.ComponentEnd();
    for(auto it = forceBegin; it != forceEnd; ++it) {
        it->f = *forcePtr++;
    }

    auto velocityBegin = velocityPool.ComponentBegin();
    auto velocityEnd   = velocityPool.ComponentEnd();
    for(auto it = velocityBegin; it != velocityEnd; ++it) {
        it->v = *velocityPtr++;
    }
}

void DataBuffers::UpdateBuffers(bismuth::Registry& registry) {
    auto& particlePool      = registry.GetComponentPool<SphereComponent>();
    auto& densityPool       = registry.GetComponentPool<DensityComponent>();
    auto& pressurePool      = registry.GetComponentPool<PressureComponent>();
    auto& forcePool         = registry.GetComponentPool<ForceComponent>();
    auto& velocityPool      = registry.GetComponentPool<VelocityComponent>();
    auto& massPool          = registry.GetComponentPool<MassComponent>();

    // Dense component arrays
    auto& positionArray     = particlePool.GetDenseComponents();
    auto& densityArray      = densityPool.GetDenseComponents();
    auto& pressureArray     = pressurePool.GetDenseComponents();
    auto& velocityArray     = velocityPool.GetDenseComponents();
    auto& forceArray        = forcePool.GetDenseComponents();
    auto& massArray         = massPool.GetDenseComponents();

    // Component locations
    auto& positionLocations = particlePool.GetComponentLocations();
    auto& densityLocations  = densityPool.GetComponentLocations();
    auto& pressureLocations = pressurePool.GetComponentLocations();
    auto& velocityLocations = velocityPool.GetComponentLocations();
    auto& forceLocations    = forcePool.GetComponentLocations();
    auto& massLocations     = massPool.GetComponentLocations();

    auto& denseEntities     = particlePool.GetDenseEntities();
    
    FillBuffer(mSphereData, positionArray);
    FillBuffer(mDensityData, densityArray);
    FillBuffer(mPressureData, pressureArray);
    FillBuffer(mVelocityData, velocityArray);
    FillBuffer(mForceData, forceArray);
    FillBuffer(mMassData, massArray);
    
    FillBuffer(mSphereLocData, positionLocations);
    FillBuffer(mDensityLocData, densityLocations);
    FillBuffer(mPressureLocData, pressureLocations);
    FillBuffer(mVelocityLocData, velocityLocations);
    FillBuffer(mForceLocData, forceLocations);
    FillBuffer(mMassLocData, massLocations);

    FillBuffer(mDenseIDs, denseEntities);

    constexpr uint32_t RESET_VALUE = 0xFFFFFFFF;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mNextPointers);
    glBufferData(GL_SHADER_STORAGE_BUFFER, denseEntities.size() * sizeof(uint32_t), nullptr, GL_DYNAMIC_COPY);
    glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &RESET_VALUE);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBucketKeys);
    glBufferData(GL_SHADER_STORAGE_BUFFER, denseEntities.size() * sizeof(uint32_t), nullptr, GL_DYNAMIC_COPY);
    glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &RESET_VALUE);
}