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
    static quartz::ButtonSystem buttonSystem;
    // static SphereDataSystem sphereDataSystem;
    // static ForceToPosSystem forceToPosSystem;
    // static PosToSpatialSystem posToSpatialSystem;

    cameraSystem.Update(mRegistry);
    guiCameraSystem.Update(mRegistry);
    buttonSystem.Update(mRegistry);
    // posToSpatialSystem.Update(mRegistry);
    // sphereDataSystem.Update(mRegistry);
    // forceToPosSystem.Update(mRegistry, deltaTime);
    
    gpuToSphereDataSystem.Update(mRegistry, mDataBuffers);
    
    uiRenderer.Update(mRegistry);

    // gInstanceRenderer.Update(mRegistry);
}
void FluidApp::Event(float deltaTime) {
    auto& mouse = mRegistry.GetSingleton<MouseStateComponent>();
    // Reset
    mouse.leftClicked  = false;
    mouse.leftPressed  = false;
    mouse.rightClicked = false;
    mouse.rightPressed = false;

    mouse.scroll = 0.0f;

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

                mouse.leftPressed = true;

            }
        }
        if(e.type == SDL_EVENT_MOUSE_MOTION) {
            mouse.position.x = e.motion.x;
            mouse.position.y = e.motion.y;
            mouse.delta.x    = e.motion.xrel;
            mouse.delta.y    = e.motion.yrel;

        }
    }
}


// Helper functions
void FluidApp::SpawnParticles(int mouseX, int mouseY) {
    auto& particleSettings = mRegistry.GetSingleton<ParticleSettingsComponent>();

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

    
    for(int x = 0; x < particleSettings.radius; x++) {
        for(int y = 0; y < particleSettings.radius; y++) {
            for(int z = 0; z < particleSettings.radius; z++) {
                mParticleSystem.CreateParticle(
                    x+worldPoint.x-1, 
                    y+worldPoint.y-1, 
                    z+worldPoint.z-1, 
                    particleSettings.mass, 
                    particleSettings.velocity
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
    // Singleton components
    mRegistry.EmplaceSingleton<MouseStateComponent>();
    mRegistry.EmplaceSingleton<ParticleSettingsComponent>();

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

    InitInterface();

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

void FluidApp::InitInterface() {
    bismuth::EntityID backgroundEntity     = mRegistry.CreateEntity();
    bismuth::EntityID labelEntity          = mRegistry.CreateEntity();

    bismuth::EntityID horizontalGui1Entity = mRegistry.CreateEntity();
    bismuth::EntityID horizontalGui2Entity = mRegistry.CreateEntity();
    bismuth::EntityID horizontalGui3Entity = mRegistry.CreateEntity();
    bismuth::EntityID horizontalGui4Entity = mRegistry.CreateEntity();
    bismuth::EntityID horizontalGui5Entity = mRegistry.CreateEntity();

    glm::vec4 backgroundColor = {0.13f, 0.13f, 0.13f, 1.0f};
    glm::vec4 fontColor       = {0.9f, 0.9f, 0.9f, 1.0f};

    float width       = 25.0f;
    float height      = 100.0f;
    float paddingLeft = 4.0f;

    float fontSize    = 3.0f;
    float fontSize2   = 2.5f;

    GuiObjectComponent background;
    background.style.Set(quartz::Properties::position,            quartz::DimensionVec2{100.0f-(width-paddingLeft), 0.0f});
    background.style.Set(quartz::Properties::width,               quartz::Dimension{width, quartz::Unit::Percent});
    background.style.Set(quartz::Properties::height,              quartz::Dimension{height, quartz::Unit::Percent});
    background.style.Set(quartz::Properties::padding_left,        quartz::Dimension{paddingLeft, quartz::Unit::Percent});
    background.style.Set(quartz::Properties::background_color,    backgroundColor);
    background.zLayer = -0.5f;


    GuiObjectComponent labelObject;
    labelObject.style.Set(quartz::Properties::height,             quartz::Dimension{3.6f, quartz::Unit::Percent});
    labelObject.style.Set(quartz::Properties::margin_top,         quartz::Dimension{3.0f, quartz::Unit::Percent});
    labelObject.style.Set(quartz::Properties::margin_bottom,      quartz::Dimension{1.5f, quartz::Unit::Percent});
    labelObject.style.Set(quartz::Properties::font_size,          quartz::Dimension{fontSize, quartz::Unit::Percent});
    labelObject.style.Set(quartz::Properties::background_color,   backgroundColor);
    labelObject.style.Set(quartz::Properties::color,              fontColor);
    labelObject.zLayer = -0.4f;

    TextMeshComponent label;
    label.content = "Particle Settings";

    // Horizontal panels
    GuiObjectComponent horizontalGui;
    horizontalGui.style.Set(quartz::Properties::height,          quartz::Dimension{5.0f, quartz::Unit::Percent});
    horizontalGui.style.Set(quartz::Properties::margin_top,      quartz::Dimension{1.5f, quartz::Unit::Percent});
    horizontalGui.style.Set(quartz::Properties::margin_bottom,   quartz::Dimension{1.5f, quartz::Unit::Percent});
    horizontalGui.style.Set(quartz::Properties::layout,          quartz::Layouts::horizontal);
    horizontalGui.style.Set(quartz::Properties::background_color, backgroundColor);
    horizontalGui.zLayer = -0.4f;

    labelObject.parentID    = backgroundEntity;
    horizontalGui.parentID = backgroundEntity;

    mRegistry.EmplaceComponent<GuiObjectComponent>(backgroundEntity, background);
    mRegistry.EmplaceComponent<GuiMeshComponent>(backgroundEntity);

    mRegistry.EmplaceComponent<GuiObjectComponent>(labelEntity, labelObject);
    mRegistry.EmplaceComponent<TextMeshComponent>(labelEntity, label);
    mRegistry.EmplaceComponent<GuiMeshComponent>(labelEntity);

    mRegistry.EmplaceComponent<GuiObjectComponent>(horizontalGui1Entity, GuiObjectComponent(horizontalGui));
    mRegistry.EmplaceComponent<GuiMeshComponent>(horizontalGui1Entity);
    mRegistry.EmplaceComponent<GuiObjectComponent>(horizontalGui2Entity, GuiObjectComponent(horizontalGui));
    mRegistry.EmplaceComponent<GuiMeshComponent>(horizontalGui2Entity);
    mRegistry.EmplaceComponent<GuiObjectComponent>(horizontalGui3Entity, GuiObjectComponent(horizontalGui));
    mRegistry.EmplaceComponent<GuiMeshComponent>(horizontalGui3Entity);
    mRegistry.EmplaceComponent<GuiObjectComponent>(horizontalGui4Entity, GuiObjectComponent(horizontalGui));
    mRegistry.EmplaceComponent<GuiMeshComponent>(horizontalGui4Entity);
    mRegistry.EmplaceComponent<GuiObjectComponent>(horizontalGui5Entity, GuiObjectComponent(horizontalGui));
    mRegistry.EmplaceComponent<GuiMeshComponent>(horizontalGui5Entity);
    
    auto& particleSettings = mRegistry.GetSingleton<ParticleSettingsComponent>();

    CreateRowGui(
        horizontalGui1Entity,
        "Radius",
        backgroundColor,
        fontColor,
        fontSize2,
        particleSettings.radius
    );

    CreateRowGui(
        horizontalGui2Entity,
        "Mass",
        backgroundColor,
        fontColor,
        fontSize2,
        particleSettings.mass
    );

    CreateRowGui(
        horizontalGui3Entity,
        "VelocityX",
        backgroundColor,
        fontColor,
        fontSize2,
        particleSettings.velocity.x
    );
    CreateRowGui(
        horizontalGui4Entity,
        "VelocityY",
        backgroundColor,
        fontColor,
        fontSize2,
        particleSettings.velocity.y
    );
    CreateRowGui(
        horizontalGui5Entity,
        "VelocityZ",
        backgroundColor,
        fontColor,
        fontSize2,
        particleSettings.velocity.z
    );

    quartz::StyleSetupSystem styleSystem(mFontManager, mWindowData.mScreenWidth, mWindowData.mScreenHeight);
    quartz::GuiVertexSetupSystem guiVertexSystem;
    styleSystem.Update(mRegistry);
    guiVertexSystem.Update(mRegistry);
}

void FluidApp::CreateRowGui(
    bismuth::EntityID  parentID, 
    std::string const& name,
    glm::vec4   const& bgColor,
    glm::vec4   const& fontColor,
    float              fontSize,
    float            & valueRef
) {
    bismuth::EntityID labelEntity    = mRegistry.CreateEntity();
    bismuth::EntityID valueEntity    = mRegistry.CreateEntity();
    bismuth::EntityID verticalEntity = mRegistry.CreateEntity();
    bismuth::EntityID buttonUEntity  = mRegistry.CreateEntity();
    bismuth::EntityID buttonDEntity  = mRegistry.CreateEntity();

    GuiObjectComponent labelObject;
    TextMeshComponent labelName;

    GuiObjectComponent valueObject;
    TextMeshComponent valueLabelName;
    
    GuiObjectComponent verticalObject;
    
    GuiObjectComponent buttonUObject;
    ButtonComponent buttonU;
    GuiObjectComponent buttonDObject;
    ButtonComponent buttonD;

    // Label
    labelObject.style.Set(quartz::Properties::width,          quartz::Dimension{50.0f, quartz::Unit::Percent});
    labelObject.style.Set(quartz::Properties::margin_top,     quartz::Dimension{30u, quartz::Unit::Pixels});
    labelObject.style.Set(quartz::Properties::padding_top,    quartz::Dimension{10u, quartz::Unit::Pixels});
    labelObject.style.Set(quartz::Properties::padding_bottom, quartz::Dimension{10u, quartz::Unit::Pixels});
    labelObject.style.Set(quartz::Properties::font_size,      quartz::Dimension{20.0f*fontSize, quartz::Unit::Percent});
    labelObject.style.Set(quartz::Properties::color,          fontColor);
    labelObject.style.Set(quartz::Properties::background_color, bgColor);
    labelObject.zLayer = -0.4f;

    labelName.content = name;

    // Buttons
    verticalObject.style.Set(quartz::Properties::width,       quartz::Dimension{10.0f, quartz::Unit::Percent});
    verticalObject.style.Set(quartz::Properties::layout,      quartz::Layouts::vertical);
    verticalObject.style.Set(quartz::Properties::background_color, glm::vec4(1.0f));
    verticalObject.zLayer = -0.35f;

    // Button Up
    buttonUObject.style.Set(quartz::Properties::height,       quartz::Dimension{50.0f, quartz::Unit::Percent});
    buttonUObject.style.Set(quartz::Properties::background_color, glm::vec4(0.0f,1.0f,0.0f,1.0f));
    buttonUObject.zLayer = -0.3f;

    buttonU.onClick = [this, &valueRef, valueEntity](){
        auto& labelPool = mRegistry.GetComponentPool<TextMeshComponent>();
        auto& label     = labelPool.GetComponent(valueEntity);
        
        valueRef++;
        label.content = std::to_string(valueRef);
    };

    // Button Down
    buttonDObject.style.Set(quartz::Properties::height,       quartz::Dimension{50.0f, quartz::Unit::Percent});
    buttonDObject.style.Set(quartz::Properties::background_color, glm::vec4(1.0f,0.0f,0.0f,1.0f));
    buttonDObject.zLayer = -0.3f;

    buttonD.onClick = [this, &valueRef, valueEntity](){
        auto& labelPool = mRegistry.GetComponentPool<TextMeshComponent>();
        auto& label     = labelPool.GetComponent(valueEntity);
        
        valueRef--;
        label.content = std::to_string(valueRef);
    };

    // Value
    valueObject.style.Set(quartz::Properties::width,          quartz::Dimension{50.0f, quartz::Unit::Percent});
    valueObject.style.Set(quartz::Properties::margin_top,     quartz::Dimension{30u, quartz::Unit::Pixels});
    valueObject.style.Set(quartz::Properties::padding_top,    quartz::Dimension{10u, quartz::Unit::Pixels});
    valueObject.style.Set(quartz::Properties::padding_bottom, quartz::Dimension{10u, quartz::Unit::Pixels});
    valueObject.style.Set(quartz::Properties::font_size,      quartz::Dimension{20.0f*fontSize, quartz::Unit::Percent});
    valueObject.style.Set(quartz::Properties::background_color, bgColor);
    valueObject.style.Set(quartz::Properties::color,          fontColor);
    valueObject.zLayer = -0.4f;
    
    valueLabelName.content = std::to_string(valueRef);

    labelObject.parentID    = parentID;
    valueObject.parentID    = parentID;
    verticalObject.parentID = parentID;

    buttonUObject.parentID = verticalEntity;
    buttonDObject.parentID = verticalEntity;

    mRegistry.EmplaceComponent<GuiObjectComponent>(labelEntity, labelObject);
    mRegistry.EmplaceComponent<GuiMeshComponent>(labelEntity);
    mRegistry.EmplaceComponent<TextMeshComponent>(labelEntity, labelName);

    mRegistry.EmplaceComponent<GuiObjectComponent>(verticalEntity, verticalObject);
    mRegistry.EmplaceComponent<GuiMeshComponent>(verticalEntity);

    mRegistry.EmplaceComponent<GuiObjectComponent>(valueEntity, valueObject);
    mRegistry.EmplaceComponent<GuiMeshComponent>(valueEntity);
    mRegistry.EmplaceComponent<TextMeshComponent>(valueEntity, valueLabelName);

    mRegistry.EmplaceComponent<GuiObjectComponent>(buttonUEntity, buttonUObject);
    mRegistry.EmplaceComponent<GuiMeshComponent>(buttonUEntity);
    mRegistry.EmplaceComponent<ButtonComponent>(buttonUEntity, buttonU);

    mRegistry.EmplaceComponent<GuiObjectComponent>(buttonDEntity, buttonDObject);
    mRegistry.EmplaceComponent<GuiMeshComponent>(buttonDEntity);
    mRegistry.EmplaceComponent<ButtonComponent>(buttonDEntity, buttonD);
}