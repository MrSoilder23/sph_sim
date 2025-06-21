#include "sapphire/systems/gpu_sphere_data_system.hpp"
#include <iostream>
#include "./quartz/core/components/camera_component.hpp"
#include "./quartz/core/components/transform_component.hpp"

#include <glm/gtc/type_ptr.hpp>

template<typename T>
void GenerateBuffers(GLuint& buffer, const std::vector<T>& data) {
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), GL_DYNAMIC_COPY);
}

GPUSphereDataSystem::GPUSphereDataSystem(bismuth::Registry& registry) {
    GLuint densityShader     = shader::CompileShader(GL_COMPUTE_SHADER, "./shaders/compute/density.glsl");
    GLuint forceShader       = shader::CompileShader(GL_COMPUTE_SHADER, "./shaders/compute/forces.glsl");
    GLuint forceToPosShader  = shader::CompileShader(GL_COMPUTE_SHADER, "./shaders/compute/force_to_pos.glsl");
    GLuint spatialHashShader = shader::CompileShader(GL_COMPUTE_SHADER, "./shaders/compute/spatial_hash.glsl");

    mRender = shader::CreateGraphicsPipeline("./shaders/ssbo_sphere_vert.glsl", "./shaders/instancedFrag.glsl");

    auto& spherePool   = registry.GetComponentPool<SphereComponent>();
    auto& densityPool  = registry.GetComponentPool<DensityComponent>();
    auto& pressurePool = registry.GetComponentPool<PressureComponent>();
    auto& forcePool    = registry.GetComponentPool<ForceComponent>();
    auto& velocityPool = registry.GetComponentPool<VelocityComponent>();
    auto& massPool     = registry.GetComponentPool<MassComponent>();

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


    mSpatialHashProgram = shader::LinkProgram(spatialHashShader);
    mDensityProgram     = shader::LinkProgram(densityShader);
    mForcesProgram      = shader::LinkProgram(forceShader);
    mPosProgram         = shader::LinkProgram(forceToPosShader);

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

    constexpr uint32_t RESET_VALUE = 0xFFFFFFFF;
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

    // Doesnt render without any vao
    glGenVertexArrays(1, &mDummyVAO);
}

void GPUSphereDataSystem::Update(bismuth::Registry& registry) {
    constexpr uint32_t RESET_VALUE = 0xFFFFFFFF;

    auto& spherePool = registry.GetComponentPool<SphereComponent>();
    auto& denseEntities = spherePool.GetDenseEntities();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mHashTable);
    glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &RESET_VALUE);
    
    ComputeSpatialHash(denseEntities);

    ComputeDensity(denseEntities);
    ComputeForces(denseEntities);
    ComputePos(denseEntities);

    Render(denseEntities, registry);
}

// Private functions
void GPUSphereDataSystem::BindSpatial() {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSphereData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mHashTable);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mNextPointers);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mBucketKeys);
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mSphereLocData);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, mDenseIDs);
}
void GPUSphereDataSystem::BindDensity() {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSphereData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mPressureData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mDensityData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mMassData);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mSphereLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, mPressureLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, mDensityLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, mMassLocData);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, mDenseIDs);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, mHashTable);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, mNextPointers);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, mBucketKeys);
}
void GPUSphereDataSystem::BindForce() {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSphereData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mVelocityData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mPressureData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mDensityData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mMassData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, mForceData);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, mSphereLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, mVelocityLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, mPressureLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, mDensityLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10,mMassLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11,mForceLocData);
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, mDenseIDs);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, mHashTable);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, mNextPointers);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, mBucketKeys);
}
void GPUSphereDataSystem::BindPosToForce() {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSphereData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mVelocityData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mForceData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mMassData);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mSphereLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, mVelocityLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, mForceLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, mMassLocData);
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, mDenseIDs);
}

void GPUSphereDataSystem::ComputeSpatialHash(const std::vector<uint32_t>& denseEntities) {
    glUseProgram(mSpatialHashProgram);

    BindSpatial();

    int uCellSize = shader::FindUniformLocation(mSpatialHashProgram, "uCellSize");
    int uHashSize = shader::FindUniformLocation(mSpatialHashProgram, "uHashSize");

    glUniform1f(uCellSize, sapphire_config::SMOOTHING_LENGTH);
    glUniform1ui(uHashSize, sapphire_config::HASH_SIZE);

    glDispatchCompute((denseEntities.size() + sapphire_config::WORKGROUP_SIZE-1) / sapphire_config::WORKGROUP_SIZE, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
void GPUSphereDataSystem::ComputeDensity(const std::vector<uint32_t>& denseEntities) {
    glUseProgram(mDensityProgram);

    BindDensity();
    
    // Uniforms
    int uStiffness   = shader::FindUniformLocation(mDensityProgram, "uStiffness");
    int uRestDensity = shader::FindUniformLocation(mDensityProgram, "uRestDensity");
    int uSmoothing   = shader::FindUniformLocation(mDensityProgram, "uSmoothingLength");

    int uCellSize = shader::FindUniformLocation(mDensityProgram, "uCellSize");
    int uHashSize = shader::FindUniformLocation(mDensityProgram, "uHashSize");

    glUniform1f(uStiffness,   sapphire_config::STIFFNESS);
    glUniform1f(uRestDensity, sapphire_config::REST_DENSITY);
    glUniform1f(uSmoothing,   sapphire_config::SMOOTHING_LENGTH);
    
    glUniform1f(uCellSize, sapphire_config::SMOOTHING_LENGTH);
    glUniform1ui(uHashSize, sapphire_config::HASH_SIZE);

    glDispatchCompute((denseEntities.size() + sapphire_config::WORKGROUP_SIZE-1) / sapphire_config::WORKGROUP_SIZE, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
void GPUSphereDataSystem::ComputeForces(const std::vector<uint32_t>& denseEntities) {
    glUseProgram(mForcesProgram);

    BindForce();
    
    // Uniforms
    int uSmoothing = shader::FindUniformLocation(mForcesProgram, "uSmoothingLength");
    int uG         = shader::FindUniformLocation(mForcesProgram, "uG");

    int uCellSize = shader::FindUniformLocation(mForcesProgram, "uCellSize");
    int uHashSize = shader::FindUniformLocation(mForcesProgram, "uHashSize");

    glUniform1f(uSmoothing, sapphire_config::SMOOTHING_LENGTH);
    glUniform1f(uG,         sapphire_config::G);

    glUniform1f(uCellSize, sapphire_config::SMOOTHING_LENGTH);
    glUniform1ui(uHashSize, sapphire_config::HASH_SIZE);

    glDispatchCompute((denseEntities.size() + sapphire_config::WORKGROUP_SIZE-1) / sapphire_config::WORKGROUP_SIZE, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
void GPUSphereDataSystem::ComputePos(const std::vector<uint32_t>& denseEntities) {
    glUseProgram(mPosProgram);

    BindPosToForce();
    
    int uTimeStep = shader::FindUniformLocation(mPosProgram, "uTimeStep");

    glUniform1f(uTimeStep, 0.01f);

    glDispatchCompute((denseEntities.size() + sapphire_config::WORKGROUP_SIZE-1) / sapphire_config::WORKGROUP_SIZE, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
void GPUSphereDataSystem::Render(const std::vector<uint32_t>& denseEntities, bismuth::Registry& registry) {
    auto& cameraPool          = registry.GetComponentPool<CameraComponent>();
    auto& cameraTransformPool = registry.GetComponentPool<TransformComponent>();

    glUseProgram(mRender);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSphereData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mSphereLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mDenseIDs);
    
    int uProjectionMatrix = shader::FindUniformLocation(mRender, "uProjectionMatrix");
    int uCameraPosition   = shader::FindUniformLocation(mRender, "uCameraPosition");

    auto& cameraProjection = cameraPool.GetDenseComponents()[0].viewProjection;
    auto& cameraPosition   = cameraTransformPool.GetDenseComponents()[0].position;

    glUniformMatrix4fv(uProjectionMatrix, 1, GL_FALSE, glm::value_ptr(cameraProjection));
    glUniformMatrix4fv(uCameraPosition,   1, GL_FALSE, glm::value_ptr(cameraPosition));

    glBindVertexArray(mDummyVAO);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArrays(GL_POINTS, 0, denseEntities.size());
}