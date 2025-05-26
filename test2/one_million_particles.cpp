// C++ standard libraries
#include <iostream>

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
#include "sapphire/test.hpp"

quartz::Engine gEngine;
bismuth::Registry gRegistry;

quartz::InstancedRendererSystem gInstanceRenderer;

void InitEntities() {
    size_t entity = gRegistry.CreateEntity();
    CameraComponent camera;
    TransformComponent transform;

    camera.aspectRatio = 640.0f/480.0f;

    gRegistry.EmplaceComponent<CameraComponent>(entity, camera);
    gRegistry.EmplaceComponent<TransformComponent>(entity, transform);

    for(int i = 0; i < 1'000'000; i++) {
        size_t sphereEntity = gRegistry.CreateEntity();
    
        gRegistry.EmplaceComponent<InstanceComponent>(sphereEntity);
        gRegistry.EmplaceComponent<SphereComponent>(sphereEntity, glm::vec4(0,i*0.5f,-10,1));
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
    static TestSystem test;

    cameraSystem.Update(gRegistry);
    test.Update(gRegistry);

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
    
    gEngine.Initialize("./config/config.json");

    GLuint shaderProgram = shader::CreateGraphicsPipeline("./shaders/sphereVert.glsl", "./shaders/instancedFrag.glsl");
    gInstanceRenderer.Init(shaderProgram);

    gEngine.SetEventCallback(Event);
    gEngine.SetSystemCallback(System);
    gEngine.SetUpdateCallback(Loop);

    gEngine.Run();
    gEngine.Shutdown();

    return 0;
}