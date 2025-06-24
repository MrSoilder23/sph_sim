#include "sapphire/application/fluid_app.hpp"

FluidApp::FluidApp(std::string configFilePath) : mWindowData(configFilePath) {
    mEngine.Initialize(configFilePath);

    // GLuint shaderProgram = shader::CreateGraphicsPipeline("./shaders/sphereVert.glsl", "./shaders/instancedFrag.glsl");
    // mInstanceRenderer.Init(shaderProgram);
    
    mEngine.SetEventCallback([this](float dt) {this->Event(dt);});
    mEngine.SetSystemCallback([this](float dt) {this->System(dt);});
    mEngine.SetUpdateCallback([this](float dt) {this->Loop(dt);});

    // Opengl Related
    InitEntities();
    
    mDataBuffers.Init(mRegistry);
}
FluidApp::~FluidApp() {
    mEngine.Shutdown();
}

void FluidApp::Run() {
    mEngine.Run();
}


// Private
void FluidApp::Loop(float deltaTime) {
    FpsCounter(deltaTime);
}
void FluidApp::System(float deltaTime) {
    static quartz::CameraSystem cameraSystem;
    static GPUSphereDataSystem gpuToSphereDataSystem(mRegistry);
    // static SphereDataSystem sphereDataSystem;
    // static ForceToPosSystem forceToPosSystem;
    // static PosToSpatialSystem posToSpatialSystem;

    cameraSystem.Update(mRegistry);

    // posToSpatialSystem.Update(mRegistry);
    // sphereDataSystem.Update(mRegistry);
    gpuToSphereDataSystem.Update(mRegistry, mDataBuffers);
    // forceToPosSystem.Update(mRegistry, deltaTime);

    // gInstanceRenderer.Update(mRegistry);
}
void FluidApp::Event(float deltaTime) {
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        // User requests quit
        if (e.type == SDL_EVENT_QUIT) {
            mEngine.Quit();
        }
        if(e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if(e.button.button == SDL_BUTTON_LEFT) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;

                mDataBuffers.SyncData(mRegistry);
                SpawnParticles(mouseX, mouseY);
                mDataBuffers.UpdateBuffers(mRegistry);

            }
        }
    }
}

// Helper functions
void FluidApp::CreateParticle(float x, float y, float z, glm::vec4 velocity) {
    size_t sphereEntity = mRegistry.CreateEntity();
            
    mRegistry.EmplaceComponent<InstanceComponent>(sphereEntity);
    mRegistry.EmplaceComponent<SphereComponent>(sphereEntity, glm::vec4(x, y, z, 1));

    mRegistry.EmplaceComponent<DensityComponent>(sphereEntity,  0.0f);
    mRegistry.EmplaceComponent<PressureComponent>(sphereEntity, 0.0f);
    mRegistry.EmplaceComponent<MassComponent>(sphereEntity,     0.3f);
    mRegistry.EmplaceComponent<ForceComponent>(sphereEntity,    glm::vec4(0.0f));
    mRegistry.EmplaceComponent<VelocityComponent>(sphereEntity, velocity);
}

void FluidApp::SpawnParticles(int mouseX, int mouseY) {
    float x = (2.0f * mouseX / mWindowData.mScreenWidth) - 1.0f;
    float y =-(2.0f * mouseY / mWindowData.mScreenHeight) + 1.0f;

    auto& cameraPool = mRegistry.GetComponentPool<CameraComponent>();
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


// Main helpers
void FluidApp::FpsCounter(float deltaTime) {
    static float smoothedFPS = 0.0f;
    static float alpha = 0.1f;  

    float fps = 1.0f/deltaTime;
    smoothedFPS = alpha * fps + (1.0f - alpha) * smoothedFPS;
    std::cout << "\33[2K\rFPS: " << static_cast<int>(smoothedFPS) << std::flush;
}

void FluidApp::InitEntities() {
    size_t entity = mRegistry.CreateEntity();
    CameraComponent camera;
    TransformComponent transform;

    camera.aspectRatio = mWindowData.mScreenWidth/mWindowData.mScreenHeight;

    mRegistry.EmplaceComponent<CameraComponent>(entity, camera);
    mRegistry.EmplaceComponent<TransformComponent>(entity, transform);

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
