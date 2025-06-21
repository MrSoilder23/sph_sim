// C++ standard libraries
#include <iostream>
#include <string>

// Third_party libraries
#include <SDL3/SDL.h>

// Own libraries
#include "bismuth/registry.hpp"
#include "quartz/core/components/camera_component.hpp"
#include "quartz/core/components/instance_component.hpp"
#include "quartz/core/components/sphere_component.hpp"
#include "quartz/core/components/transform_component.hpp"
#include "quartz/core/systems/camera_system.hpp"
#include "quartz/engine.hpp"
#include "quartz/graphics/instanced_renderer_system.hpp"
#include "quartz/graphics/shader.hpp"

#include "sapphire/components/density_component.hpp"
#include "sapphire/components/force_component.hpp"
#include "sapphire/components/mass_component.hpp"
#include "sapphire/components/pressure_component.hpp"
#include "sapphire/components/velocity_component.hpp"
#include "sapphire/components/energy_component.hpp"
#include "sapphire/utility/config.hpp"
#include "sapphire/utility/window_data.hpp"

#include "sapphire/systems/sphere_data_system.hpp"
#include "sapphire/systems/force_to_pos_system.hpp"
#include "sapphire/systems/pos_to_spatial_system.hpp"

#include "sapphire/systems/gpu_sphere_data_system.hpp"
#include "sapphire/utility/data_buffers.hpp"

quartz::Engine gEngine;
bismuth::Registry gRegistry;

std::string gConfigFilePath = "./config/config.json";
WindowData gWindowData(gConfigFilePath);

quartz::InstancedRendererSystem gInstanceRenderer;
DataBuffers gParticleSSBO;

void CreateParticle(float x, float y, float z, glm::vec4 velocity) {
    size_t sphereEntity = gRegistry.CreateEntity();
            
    gRegistry.EmplaceComponent<InstanceComponent>(sphereEntity);
    gRegistry.EmplaceComponent<SphereComponent>(sphereEntity, glm::vec4(x, y, z, 1));

    gRegistry.EmplaceComponent<DensityComponent>(sphereEntity,  0.0f);
    gRegistry.EmplaceComponent<PressureComponent>(sphereEntity, 0.0f);
    gRegistry.EmplaceComponent<MassComponent>(sphereEntity,     0.5f);
    gRegistry.EmplaceComponent<ForceComponent>(sphereEntity,    glm::vec4(0.0f));
    gRegistry.EmplaceComponent<VelocityComponent>(sphereEntity, velocity);
}

void InitEntities() {
    size_t entity = gRegistry.CreateEntity();
    CameraComponent camera;
    TransformComponent transform;

    camera.aspectRatio = gWindowData.mScreenWidth/gWindowData.mScreenHeight;

    gRegistry.EmplaceComponent<CameraComponent>(entity, camera);
    gRegistry.EmplaceComponent<TransformComponent>(entity, transform);

    int amountX = 25;
    int amountY = 25;
    int amountZ = 25;

    float coordOffsetX = (amountX/2)*sapphire_config::INITIAL_SPACING;
    float coordOffsetY = (amountY/2)*sapphire_config::INITIAL_SPACING;
    float coordOffsetZ = (amountZ/2)*sapphire_config::INITIAL_SPACING;

    for(int x = 0; x < amountX; x++) {
        for(int y = 0; y < amountY; y++) {
            for(int z = 0; z < amountZ; z++) {
                CreateParticle(
                    x*sapphire_config::INITIAL_SPACING - coordOffsetX, 
                    y*sapphire_config::INITIAL_SPACING - coordOffsetY, 
                    z*sapphire_config::INITIAL_SPACING - coordOffsetZ - 40,
                    glm::vec4(0.0f)
                );
            }
        }
    }
}

void SpawnParticles(int mouseX, int mouseY) {
    float x = (2.0f * mouseX / gWindowData.mScreenWidth) - 1.0f;
    float y =-(2.0f * mouseY / gWindowData.mScreenHeight) + 1.0f;

    auto& cameraPool = gRegistry.GetComponentPool<CameraComponent>();
    auto& camera = cameraPool.GetComponent(0);

    glm::vec4 rayClip(x,y,-1.0f, 1.0f);
    glm::vec4 rayEye = glm::inverse(camera.projectionMatrix) * rayClip;
    rayEye.z = -1.0f;
    rayEye.w =  0.0f;

    glm::vec3 rayWorld = glm::vec3(glm::inverse(camera.viewMatrix) * rayEye);
    rayWorld = glm::normalize(rayWorld);

    float t = -40.0f / rayWorld.z;
    glm::vec3 worldPoint = glm::vec3(0.0f) + t * rayWorld;

    for(int x = 0; x < 3; x++) {
        for(int y = 0; y < 3; y++) {
            for(int z = 0; z < 3; z++) {
                CreateParticle(x+worldPoint.x-1, y+worldPoint.y-1, z+worldPoint.z-1, glm::vec4(-8.0f,0.0f,0.0f, 0.0f));
            }
        }
    }
}

void SynchroniseData() {
    auto& particlePool     = gRegistry.GetComponentPool<SphereComponent>();
    auto& densityPool      = gRegistry.GetComponentPool<DensityComponent>();
    auto& pressurePool     = gRegistry.GetComponentPool<PressureComponent>();
    auto& forcePool        = gRegistry.GetComponentPool<ForceComponent>();
    auto& velocityPool     = gRegistry.GetComponentPool<VelocityComponent>();

    auto& denseParticleIDs = particlePool.GetDenseEntities();

    const size_t sphereSize   = denseParticleIDs.size() * sizeof(SphereComponent);
    const size_t densitySize  = denseParticleIDs.size() * sizeof(DensityComponent);
    const size_t pressureSize = denseParticleIDs.size() * sizeof(PressureComponent);
    const size_t forceSize    = denseParticleIDs.size() * sizeof(ForceComponent);
    const size_t velocitySize = denseParticleIDs.size() * sizeof(VelocityComponent);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gParticleSSBO.mSphereData);
    glm::vec4* posPtr = static_cast<glm::vec4*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sphereSize, GL_MAP_READ_BIT));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gParticleSSBO.mDensityData);
    float* densityPtr = static_cast<float*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, densitySize, GL_MAP_READ_BIT));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gParticleSSBO.mPressureData);
    float* pressurePtr = static_cast<float*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, pressureSize, GL_MAP_READ_BIT));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gParticleSSBO.mForceData);
    glm::vec4* forcePtr = static_cast<glm::vec4*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, forceSize, GL_MAP_READ_BIT));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gParticleSSBO.mVelocityData);
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

template<typename T>
void FillBuffer(GLuint& buffer, const std::vector<T>& data) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), GL_DYNAMIC_COPY);
}


void UpdateBuffers() {
    auto& particlePool      = gRegistry.GetComponentPool<SphereComponent>();
    auto& densityPool       = gRegistry.GetComponentPool<DensityComponent>();
    auto& pressurePool      = gRegistry.GetComponentPool<PressureComponent>();
    auto& forcePool         = gRegistry.GetComponentPool<ForceComponent>();
    auto& velocityPool      = gRegistry.GetComponentPool<VelocityComponent>();
    auto& massPool          = gRegistry.GetComponentPool<MassComponent>();

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
    
    FillBuffer(gParticleSSBO.mSphereData, positionArray);
    FillBuffer(gParticleSSBO.mDensityData, densityArray);
    FillBuffer(gParticleSSBO.mPressureData, pressureArray);
    FillBuffer(gParticleSSBO.mVelocityData, velocityArray);
    FillBuffer(gParticleSSBO.mForceData, forceArray);
    FillBuffer(gParticleSSBO.mMassData, massArray);
    
    FillBuffer(gParticleSSBO.mSphereLocData, positionLocations);
    FillBuffer(gParticleSSBO.mDensityLocData, densityLocations);
    FillBuffer(gParticleSSBO.mPressureLocData, pressureLocations);
    FillBuffer(gParticleSSBO.mVelocityLocData, velocityLocations);
    FillBuffer(gParticleSSBO.mForceLocData, forceLocations);
    FillBuffer(gParticleSSBO.mMassLocData, massLocations);

    FillBuffer(gParticleSSBO.mDenseIDs, denseEntities);
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
}

void Event(float deltaTime) {
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        // User requests quit
        if (e.type == SDL_EVENT_QUIT) {
            gEngine.Quit();
        }
        if(e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if(e.button.button == SDL_BUTTON_LEFT) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;

                SynchroniseData();
                SpawnParticles(mouseX, mouseY);
                UpdateBuffers();

            }
        }
    }
}

void System(float deltaTime) {
    static quartz::CameraSystem cameraSystem;
    static SphereDataSystem sphereDataSystem;
    static ForceToPosSystem forceToPosSystem;
    static PosToSpatialSystem posToSpatialSystem;
    static GPUSphereDataSystem gpuToSphereDataSystem(gRegistry);

    cameraSystem.Update(gRegistry);

    // posToSpatialSystem.Update(gRegistry);
    // sphereDataSystem.Update(gRegistry);
    gpuToSphereDataSystem.Update(gRegistry, gParticleSSBO);
    // forceToPosSystem.Update(gRegistry, deltaTime);

    // gInstanceRenderer.Update(gRegistry);
}

void FpsCounter(float deltaTime) {
    static float smoothedFPS = 0.0f;
    static float alpha = 0.1f;  

    float fps = 1.0f/deltaTime;
    smoothedFPS = alpha * fps + (1.0f - alpha) * smoothedFPS;
    std::cout << "\33[2K\rFPS: " << static_cast<int>(smoothedFPS) << std::flush;
}

void Loop(float deltaTime) {
    FpsCounter(deltaTime);
}

void Initialize() {
    // Engine
    gEngine.Initialize(gConfigFilePath);

    GLuint shaderProgram = shader::CreateGraphicsPipeline("./shaders/sphereVert.glsl", "./shaders/instancedFrag.glsl");
    gInstanceRenderer.Init(shaderProgram);
    
    gEngine.SetEventCallback(Event);
    gEngine.SetSystemCallback(System);
    gEngine.SetUpdateCallback(Loop);

    // Opengl Related
    InitEntities();
    
    gParticleSSBO.Init(gRegistry);
}


int main(int argc, char* argv[]) {
    Initialize();

    gEngine.Run();

    gEngine.Shutdown();

    return 0;
}