#include "sapphire/systems/gpu_sphere_data_system.hpp"
#include <iostream>
#include "./quartz/core/components/camera_component.hpp"
#include "./quartz/core/components/transform_component.hpp"

#include <glm/gtc/type_ptr.hpp>

GPUSphereDataSystem::GPUSphereDataSystem(bismuth::Registry& registry) {
    GLuint densityShader     = shader::CompileShader(GL_COMPUTE_SHADER, "./shaders/compute/density.glsl");
    GLuint forceShader       = shader::CompileShader(GL_COMPUTE_SHADER, "./shaders/compute/forces.glsl");
    GLuint forceToPosShader  = shader::CompileShader(GL_COMPUTE_SHADER, "./shaders/compute/force_to_pos.glsl");
    GLuint spatialHashShader = shader::CompileShader(GL_COMPUTE_SHADER, "./shaders/compute/spatial_hash.glsl");

    mRender = shader::CreateGraphicsPipeline("./shaders/ssbo_sphere_vert.glsl", "./shaders/instancedFrag.glsl");

    mSpatialHashProgram = shader::LinkProgram(spatialHashShader);
    mDensityProgram     = shader::LinkProgram(densityShader);
    mForcesProgram      = shader::LinkProgram(forceShader);
    mPosProgram         = shader::LinkProgram(forceToPosShader);

    // Doesnt render without any vao
    glGenVertexArrays(1, &mDummyVAO);
}

void GPUSphereDataSystem::Update(bismuth::Registry& registry, DataBuffers& dataBuffer) {
    constexpr uint32_t RESET_VALUE = 0xFFFFFFFF;

    auto& spherePool = registry.GetComponentPool<SphereComponent>();
    auto& denseEntities = spherePool.GetDenseEntities();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, dataBuffer.mHashTable);
    glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &RESET_VALUE);
    
    ComputeSpatialHash(denseEntities, dataBuffer);

    ComputeDensity(denseEntities, dataBuffer);
    ComputeForces(denseEntities, dataBuffer);
    ComputePos(denseEntities, dataBuffer);

    Render(denseEntities, registry, dataBuffer);
}

// Private functions
void GPUSphereDataSystem::BindSpatial(DataBuffers& dataBuffer) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, dataBuffer.mSphereData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, dataBuffer.mHashTable);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, dataBuffer.mNextPointers);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, dataBuffer.mBucketKeys);
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, dataBuffer.mSphereLocData);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, dataBuffer.mDenseIDs);
}
void GPUSphereDataSystem::BindDensity(DataBuffers& dataBuffer) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, dataBuffer.mSphereData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, dataBuffer.mPressureData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, dataBuffer.mDensityData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, dataBuffer.mMassData);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, dataBuffer.mSphereLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, dataBuffer.mPressureLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, dataBuffer.mDensityLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, dataBuffer.mMassLocData);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, dataBuffer.mDenseIDs);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, dataBuffer.mHashTable);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, dataBuffer.mNextPointers);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, dataBuffer.mBucketKeys);
}
void GPUSphereDataSystem::BindForce(DataBuffers& dataBuffer) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, dataBuffer.mSphereData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, dataBuffer.mVelocityData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, dataBuffer.mPressureData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, dataBuffer.mDensityData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, dataBuffer.mMassData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, dataBuffer.mForceData);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, dataBuffer.mSphereLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, dataBuffer.mVelocityLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, dataBuffer.mPressureLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, dataBuffer.mDensityLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10,dataBuffer.mMassLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11,dataBuffer.mForceLocData);
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, dataBuffer.mDenseIDs);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, dataBuffer.mHashTable);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, dataBuffer.mNextPointers);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, dataBuffer.mBucketKeys);
}
void GPUSphereDataSystem::BindPosToForce(DataBuffers& dataBuffer) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, dataBuffer.mSphereData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, dataBuffer.mVelocityData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, dataBuffer.mForceData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, dataBuffer.mMassData);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, dataBuffer.mSphereLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, dataBuffer.mVelocityLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, dataBuffer.mForceLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, dataBuffer.mMassLocData);
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, dataBuffer.mDenseIDs);
}

void GPUSphereDataSystem::ComputeSpatialHash(const std::vector<uint32_t>& denseEntities, DataBuffers& dataBuffer) {
    glUseProgram(mSpatialHashProgram);

    BindSpatial(dataBuffer);

    int uCellSize = shader::FindUniformLocation(mSpatialHashProgram, "uCellSize");
    int uHashSize = shader::FindUniformLocation(mSpatialHashProgram, "uHashSize");

    glUniform1f(uCellSize, sapphire_config::SMOOTHING_LENGTH);
    glUniform1ui(uHashSize, sapphire_config::HASH_SIZE);

    glDispatchCompute((denseEntities.size() + sapphire_config::WORKGROUP_SIZE-1) / sapphire_config::WORKGROUP_SIZE, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
void GPUSphereDataSystem::ComputeDensity(const std::vector<uint32_t>& denseEntities, DataBuffers& dataBuffer) {
    glUseProgram(mDensityProgram);

    BindDensity(dataBuffer);
    
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
void GPUSphereDataSystem::ComputeForces(const std::vector<uint32_t>& denseEntities, DataBuffers& dataBuffer) {
    glUseProgram(mForcesProgram);

    BindForce(dataBuffer);
    
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
void GPUSphereDataSystem::ComputePos(const std::vector<uint32_t>& denseEntities, DataBuffers& dataBuffer) {
    glUseProgram(mPosProgram);

    BindPosToForce(dataBuffer);
    
    int uTimeStep = shader::FindUniformLocation(mPosProgram, "uTimeStep");

    glUniform1f(uTimeStep, 0.01f);

    glDispatchCompute((denseEntities.size() + sapphire_config::WORKGROUP_SIZE-1) / sapphire_config::WORKGROUP_SIZE, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
void GPUSphereDataSystem::Render(const std::vector<uint32_t>& denseEntities, bismuth::Registry& registry, DataBuffers& dataBuffer) {
    auto& cameraPool          = registry.GetComponentPool<CameraComponent>();
    auto& cameraTransformPool = registry.GetComponentPool<TransformComponent>();

    glUseProgram(mRender);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, dataBuffer.mSphereData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, dataBuffer.mSphereLocData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, dataBuffer.mDenseIDs);
    
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