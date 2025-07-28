#include "sapphire/application/fluid_app.hpp"

FluidApp::FluidApp(std::string configFilePath) : mWindowData(configFilePath), mParticleSystem(mRegistry) {
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
    static quartz::GuiCameraSystem guiCameraSystem;
    static GPUSphereDataSystem gpuToSphereDataSystem(mRegistry);
    static quartz::UiRendererSystem uiRenderer;
    // static SphereDataSystem sphereDataSystem;
    // static ForceToPosSystem forceToPosSystem;
    // static PosToSpatialSystem posToSpatialSystem;

    cameraSystem.Update(mRegistry);
    guiCameraSystem.Update(mRegistry);

    // posToSpatialSystem.Update(mRegistry);
    // sphereDataSystem.Update(mRegistry);
    // forceToPosSystem.Update(mRegistry, deltaTime);
    
    gpuToSphereDataSystem.Update(mRegistry, mDataBuffers);
    
    uiRenderer.Update(mRegistry);

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
                mParticleSystem.CreateParticle(
                    x+worldPoint.x-1, 
                    y+worldPoint.y-1, 
                    z+worldPoint.z-1, 
                    0.3f, 
                    glm::vec4(-8.0f,0.0f,0.0f, 0.0f)
                );
            }
        }
    }
}

void FluidApp::FpsCounter(float deltaTime) {
    static float smoothedFPS = 0.0f;
    static float alpha = 0.1f;  

    float fps = 1.0f/deltaTime;
    smoothedFPS = alpha * fps + (1.0f - alpha) * smoothedFPS;
    std::cout << "\33[2K\rFPS: " << static_cast<int>(smoothedFPS) << std::flush;
}

void FluidApp::InitEntities() {
    // Camera
    size_t cameraEntity = mRegistry.CreateEntity();
    CameraComponent camera;
    TransformComponent transform;

    camera.aspectRatio = mWindowData.mScreenWidth/mWindowData.mScreenHeight;

    mRegistry.EmplaceComponent<CameraComponent>(cameraEntity, camera);
    mRegistry.EmplaceComponent<TransformComponent>(cameraEntity, transform);

    // Gui Camera
    uint32_t guiCameraEntity = mRegistry.CreateEntity();

    GuiCameraComponent guiCamera;
    guiCamera.height = mWindowData.mScreenHeight;
    guiCamera.width  = mWindowData.mScreenWidth;

    mRegistry.EmplaceComponent<GuiCameraComponent>(guiCameraEntity, guiCamera);

    // Gui Object
    uint32_t guiObjectEntity  = mRegistry.CreateEntity();
    uint32_t guiObjectEntity1 = mRegistry.CreateEntity();
    uint32_t guiObjectEntity2 = mRegistry.CreateEntity();
    uint32_t guiObjectEntity3 = mRegistry.CreateEntity();
    uint32_t guiObjectEntity4 = mRegistry.CreateEntity();
    
    GuiObjectComponent guiObject;
    guiObject.style.Set(quartz::Properties::position, glm::vec2(mWindowData.mScreenWidth-200, 0.0f));
    guiObject.style.Set(quartz::Properties::width,    200u);
    guiObject.style.Set(quartz::Properties::height,   static_cast<unsigned int>(mWindowData.mScreenHeight));
    
    guiObject.style.Set(quartz::Properties::padding_left, 30u);

    guiObject.style.Set(quartz::Properties::background_color, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    guiObject.zLayer = -0.5f;

    GuiObjectComponent guiObject1;
    guiObject1.style.Set(quartz::Properties::width,    200u);
    guiObject1.style.Set(quartz::Properties::height,   50u);
    guiObject1.style.Set(quartz::Properties::background_color, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
    guiObject1.zLayer = -0.4f;

    GuiObjectComponent guiObject2;
    guiObject2.style.Set(quartz::Properties::width,    200u);
    guiObject2.style.Set(quartz::Properties::height,   50u);

    guiObject2.style.Set(quartz::Properties::margin_bottom,  30u);
    guiObject2.style.Set(quartz::Properties::padding_top,    30u);
    guiObject2.style.Set(quartz::Properties::padding_bottom, 30u);
    
    guiObject2.style.Set(quartz::Properties::background_color, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
    guiObject2.zLayer = -0.4f;

    GuiObjectComponent guiObject3;
    guiObject3.style.Set(quartz::Properties::width,    200u);
    guiObject3.style.Set(quartz::Properties::height,   50u);
    guiObject3.style.Set(quartz::Properties::background_color, glm::vec4(1.0f, 0.5f, 1.0f, 1.0f));
    guiObject3.zLayer = -0.4f;

    GuiObjectComponent guiObject4;
    guiObject4.style.Set(quartz::Properties::background_color, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    guiObject4.style.Set(quartz::Properties::color,            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    guiObject4.style.Set(quartz::Properties::height,           50u);
    guiObject4.style.Set(quartz::Properties::padding_left,     10u);
    guiObject4.zLayer = -0.3f;

    TextMeshComponent textMesh;
    textMesh.content = "W. abcd123";

    guiObject.childrenIDs.push_back(guiObjectEntity1);
    guiObject.childrenIDs.push_back(guiObjectEntity2);
    guiObject.childrenIDs.push_back(guiObjectEntity3);

    guiObject.childrenIDs.push_back(guiObjectEntity4);

    mRegistry.EmplaceComponent<GuiObjectComponent>(guiObjectEntity, guiObject);
    mRegistry.EmplaceComponent<GuiMeshComponent>(guiObjectEntity);

    mRegistry.EmplaceComponent<GuiObjectComponent>(guiObjectEntity1, guiObject1);
    mRegistry.EmplaceComponent<GuiMeshComponent>(guiObjectEntity1);

    mRegistry.EmplaceComponent<GuiObjectComponent>(guiObjectEntity2, guiObject2);
    mRegistry.EmplaceComponent<GuiMeshComponent>(guiObjectEntity2);

    mRegistry.EmplaceComponent<GuiObjectComponent>(guiObjectEntity3, guiObject3);
    mRegistry.EmplaceComponent<GuiMeshComponent>(guiObjectEntity3);

    mRegistry.EmplaceComponent<GuiObjectComponent>(guiObjectEntity4, guiObject4);
    mRegistry.EmplaceComponent<GuiMeshComponent>(guiObjectEntity4);
    mRegistry.EmplaceComponent<TextMeshComponent>(guiObjectEntity4, textMesh);

    quartz::StyleSetupSystem styleSystem(mFontManager);
    quartz::GuiVertexSetupSystem guiVertexSystem;
    styleSystem.Update(mRegistry);
    guiVertexSystem.Update(mRegistry);

    // Initial cube
    int amountX = 25;
    int amountY = 25;
    int amountZ = 25;

    float coordOffsetX = (amountX/2)*sapphire_config::INITIAL_SPACING;
    float coordOffsetY = (amountY/2)*sapphire_config::INITIAL_SPACING;
    float coordOffsetZ = (amountZ/2)*sapphire_config::INITIAL_SPACING;

    for(int x = 0; x < amountX; x++) {
        for(int y = 0; y < amountY; y++) {
            for(int z = 0; z < amountZ; z++) {
                mParticleSystem.CreateParticle(
                    x*sapphire_config::INITIAL_SPACING - coordOffsetX, 
                    y*sapphire_config::INITIAL_SPACING - coordOffsetY, 
                    z*sapphire_config::INITIAL_SPACING - coordOffsetZ - 40,
                    0.3f,
                    glm::vec4(0.0f)
                );
            }
        }
    }
}
