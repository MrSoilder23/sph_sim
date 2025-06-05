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

quartz::Engine gEngine;
bismuth::Registry gRegistry;

std::string gConfigFilePath = "./config/config.json";
WindowData gWindowData(gConfigFilePath);

quartz::InstancedRendererSystem gInstanceRenderer;

void CreateParticle(float x, float y, float z, glm::vec3 velocity) {
    size_t sphereEntity = gRegistry.CreateEntity();
            
    gRegistry.EmplaceComponent<InstanceComponent>(sphereEntity);
    gRegistry.EmplaceComponent<SphereComponent>(sphereEntity, glm::vec4(x, y, z, 1));

    gRegistry.EmplaceComponent<DensityComponent>(sphereEntity,  0.0f);
    gRegistry.EmplaceComponent<PressureComponent>(sphereEntity, 0.0f);
    gRegistry.EmplaceComponent<EnergyComponent>(sphereEntity,   0.0f);
    gRegistry.EmplaceComponent<MassComponent>(sphereEntity,     1.0f);
    gRegistry.EmplaceComponent<ForceComponent>(sphereEntity,    glm::vec3(0.0f));
    gRegistry.EmplaceComponent<VelocityComponent>(sphereEntity, velocity);
}

void InitEntities() {
    size_t entity = gRegistry.CreateEntity();
    CameraComponent camera;
    TransformComponent transform;

    camera.aspectRatio = gWindowData.mScreenWidth/gWindowData.mScreenHeight;

    gRegistry.EmplaceComponent<CameraComponent>(entity, camera);
    gRegistry.EmplaceComponent<TransformComponent>(entity, transform);

    int amountX = 100;
    int amountY = 10;
    int amountZ = 10;

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
                    glm::vec3(0.0f)
                );
            }
        }
    }
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

                float x = (2.0f * e.button.x / gWindowData.mScreenWidth) - 1.0f;
                float y =-(2.0f * e.button.y / gWindowData.mScreenHeight) + 1.0f;

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
                            CreateParticle(x+worldPoint.x-1, y+worldPoint.y-1, z+worldPoint.z-1, glm::vec3(-8.0f,0.0f,0.0f));
                        }
                    }
                }
            }
        }
    }
}

void System(float deltaTime) {
    static quartz::CameraSystem cameraSystem;
    static SphereDataSystem sphereDataSystem;
    static ForceToPosSystem forceToPosSystem;
    static PosToSpatialSystem posToSpatialSystem;

    cameraSystem.Update(gRegistry);

    posToSpatialSystem.Update(gRegistry);
    sphereDataSystem.Update(gRegistry);
    forceToPosSystem.Update(gRegistry, deltaTime);

    gInstanceRenderer.Update(gRegistry);
}

void FpsCounter(float deltaTime) {
    static float smoothedFPS = 0.0f;
    static float alpha = 0.1f;  

    float fps = 1.0f/deltaTime;
    smoothedFPS = alpha * fps + (1.0f - alpha) * smoothedFPS;
    std::cout << "\rFPS: " << static_cast<int>(smoothedFPS) << std::flush;
}

void Loop(float deltaTime) {
    FpsCounter(deltaTime);
}

int main(int argc, char* argv[]) {
    InitEntities();
    
    gEngine.Initialize(gConfigFilePath);

    GLuint shaderProgram = shader::CreateGraphicsPipeline("./shaders/sphereVert.glsl", "./shaders/instancedFrag.glsl");
    gInstanceRenderer.Init(shaderProgram);

    gEngine.SetEventCallback(Event);
    gEngine.SetSystemCallback(System);
    gEngine.SetUpdateCallback(Loop);

    gEngine.Run();
    gEngine.Shutdown();

    return 0;
}