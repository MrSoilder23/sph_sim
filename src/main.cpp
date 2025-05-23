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

quartz::Engine gEngine;
bismuth::Registry gRegistry;

quartz::InstancedRendererSystem gInstanceRenderer;

std::string gConfigFilePath = "./config/config.json";

void InitEntities() {
    size_t entity = gRegistry.CreateEntity();
    CameraComponent camera;
    TransformComponent transform;

    std::ifstream configFile(gConfigFilePath);
    if (!configFile) {
        std::cerr << "Failed to open configuration file." << std::endl;
        exit(1);
    }

    nlohmann::json config;
    configFile >> config;

    float screenWidth = config["window"]["width"];
    float screenHeight = config["window"]["height"];

    camera.aspectRatio = screenWidth/screenHeight;

    gRegistry.EmplaceComponent<CameraComponent>(entity, camera);
    gRegistry.EmplaceComponent<TransformComponent>(entity, transform);

    // Initial data
    float spacing = 0.5f;

    float coordOffset = 5*spacing;

    for(int x = 0; x < 10; x++) {
        for(int y = 0; y < 10; y++) {
            size_t sphereEntity = gRegistry.CreateEntity();
        
            gRegistry.EmplaceComponent<InstanceComponent>(sphereEntity);
            gRegistry.EmplaceComponent<SphereComponent>(sphereEntity, glm::vec4(x*spacing-coordOffset,y*spacing-coordOffset,-10,1));

            gRegistry.EmplaceComponent<DensityComponent>(sphereEntity);
            gRegistry.EmplaceComponent<PressureComponent>(sphereEntity);
            gRegistry.EmplaceComponent<MassComponent>(sphereEntity,     10.0f);
            gRegistry.EmplaceComponent<ForceComponent>(sphereEntity,    glm::vec3(0.0f));
            gRegistry.EmplaceComponent<VelocityComponnet>(sphereEntity, glm::vec3(0.0f));
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
    }
}

void System(float deltaTime) {
    static quartz::CameraSystem cameraSystem;

    cameraSystem.Update(gRegistry);

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