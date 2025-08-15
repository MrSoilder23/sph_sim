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
    bismuth::EntityID cameraEntity = mRegistry.CreateEntity();
    CameraComponent camera;
    TransformComponent transform;

    camera.aspectRatio = mWindowData.mScreenWidth/mWindowData.mScreenHeight;

    mRegistry.EmplaceComponent<CameraComponent>(cameraEntity, camera);
    mRegistry.EmplaceComponent<TransformComponent>(cameraEntity, transform);

    // Gui Camera
    bismuth::EntityID guiCameraEntity = mRegistry.CreateEntity();

    GuiCameraComponent guiCamera;
    guiCamera.height = mWindowData.mScreenHeight;
    guiCamera.width  = mWindowData.mScreenWidth;

    mRegistry.EmplaceComponent<GuiCameraComponent>(guiCameraEntity, guiCamera);

    // Gui Object
    bismuth::EntityID guiObjectEntity  = mRegistry.CreateEntity();
    bismuth::EntityID guiObjectEntity1 = mRegistry.CreateEntity();
    bismuth::EntityID guiObjectEntity2 = mRegistry.CreateEntity();
    bismuth::EntityID guiObjectEntity3 = mRegistry.CreateEntity();
    bismuth::EntityID guiObjectEntity4 = mRegistry.CreateEntity();
    
    GuiObjectComponent guiObject;
    guiObject.style.Set(quartz::Properties::position, quartz::DimensionVec2{30.0f, 0.0f});
    guiObject.style.Set(quartz::Properties::width,    quartz::Dimension{200u, quartz::Unit::Pixels});
    guiObject.style.Set(quartz::Properties::height,   quartz::Dimension{mWindowData.mScreenHeight, quartz::Unit::Pixels});
    
    guiObject.style.Set(quartz::Properties::padding_left, quartz::Dimension{30u, quartz::Unit::Pixels});

    guiObject.style.Set(quartz::Properties::background_color, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    guiObject.zLayer = -0.5f;

    GuiObjectComponent guiObject1;
    guiObject1.style.Set(quartz::Properties::width,    quartz::Dimension{200u, quartz::Unit::Pixels});
    guiObject1.style.Set(quartz::Properties::height,   quartz::Dimension{30.0f, quartz::Unit::Percent});
    guiObject1.style.Set(quartz::Properties::background_color, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
    guiObject1.zLayer = -0.4f;

    GuiObjectComponent guiObject2;
    guiObject2.style.Set(quartz::Properties::width,    quartz::Dimension{200u, quartz::Unit::Pixels});
    guiObject2.style.Set(quartz::Properties::height,   quartz::Dimension{50u, quartz::Unit::Pixels});

    guiObject2.style.Set(quartz::Properties::margin_top,  quartz::Dimension{30u, quartz::Unit::Pixels});
    guiObject2.style.Set(quartz::Properties::padding_top,    quartz::Dimension{30u, quartz::Unit::Pixels});
    guiObject2.style.Set(quartz::Properties::padding_bottom, quartz::Dimension{30u, quartz::Unit::Pixels});
    
    guiObject2.style.Set(quartz::Properties::background_color, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
    guiObject2.zLayer = -0.4f;

    GuiObjectComponent guiObject3;
    guiObject3.style.Set(quartz::Properties::width,    quartz::Dimension{200u, quartz::Unit::Pixels});
    guiObject3.style.Set(quartz::Properties::height,   quartz::Dimension{50u, quartz::Unit::Pixels});
    guiObject3.style.Set(quartz::Properties::background_color, glm::vec4(1.0f, 0.5f, 1.0f, 1.0f));
    guiObject3.zLayer = -0.4f;

    GuiObjectComponent guiObject4;
    guiObject4.style.Set(quartz::Properties::background_color, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    guiObject4.style.Set(quartz::Properties::color,            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    guiObject4.style.Set(quartz::Properties::height,           quartz::Dimension{50u, quartz::Unit::Pixels});
    guiObject4.style.Set(quartz::Properties::padding_left,     quartz::Dimension{10u, quartz::Unit::Pixels});
    guiObject4.style.Set(quartz::Properties::padding_top,      quartz::Dimension{30u, quartz::Unit::Pixels});
    guiObject4.zLayer = -0.3f;

    TextMeshComponent textMesh;
    textMesh.content = "W. abcd123q";

    guiObject1.parentID = guiObjectEntity;
    guiObject2.parentID = guiObjectEntity;
    guiObject3.parentID = guiObjectEntity;
    guiObject4.parentID = guiObjectEntity;

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

    quartz::StyleSetupSystem styleSystem(mFontManager, mWindowData.mScreenWidth, mWindowData.mScreenHeight);
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
