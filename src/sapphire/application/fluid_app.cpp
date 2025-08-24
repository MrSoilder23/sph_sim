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
    bismuth::EntityID backgroundEntity = mRegistry.CreateEntity();
    bismuth::EntityID labelEntity      = mRegistry.CreateEntity();

    bismuth::EntityID horizontalGui1Entity = mRegistry.CreateEntity();
    bismuth::EntityID horizontalGui2Entity = mRegistry.CreateEntity();
    bismuth::EntityID horizontalGui3Entity = mRegistry.CreateEntity();

    bismuth::EntityID radiusEntity   = mRegistry.CreateEntity();
    bismuth::EntityID massEntity     = mRegistry.CreateEntity();
    bismuth::EntityID velocityEntity = mRegistry.CreateEntity();
    
    bismuth::EntityID radiusButtonEntity   = mRegistry.CreateEntity();
    bismuth::EntityID massButtonEntity     = mRegistry.CreateEntity();
    bismuth::EntityID velocityButtonEntity = mRegistry.CreateEntity();

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
    GuiObjectComponent horizontalGui1;
    horizontalGui1.style.Set(quartz::Properties::height,          quartz::Dimension{5.0f, quartz::Unit::Percent});
    horizontalGui1.style.Set(quartz::Properties::margin_top,      quartz::Dimension{1.5f, quartz::Unit::Percent});
    horizontalGui1.style.Set(quartz::Properties::margin_bottom,   quartz::Dimension{1.5f, quartz::Unit::Percent});
    horizontalGui1.style.Set(quartz::Properties::layout,          quartz::Layouts::horizontal);
    horizontalGui1.style.Set(quartz::Properties::background_color, backgroundColor);

    GuiObjectComponent horizontalGui2;
    horizontalGui2.style.Set(quartz::Properties::height,          quartz::Dimension{5.0f, quartz::Unit::Percent});
    horizontalGui2.style.Set(quartz::Properties::padding_top,     quartz::Dimension{1.5f, quartz::Unit::Percent});
    horizontalGui2.style.Set(quartz::Properties::padding_bottom,  quartz::Dimension{1.5f, quartz::Unit::Percent});
    horizontalGui2.style.Set(quartz::Properties::layout,          quartz::Layouts::horizontal);
    horizontalGui2.style.Set(quartz::Properties::background_color, backgroundColor);

    GuiObjectComponent horizontalGui3;
    horizontalGui3.style.Set(quartz::Properties::height,          quartz::Dimension{5.0f, quartz::Unit::Percent});
    horizontalGui3.style.Set(quartz::Properties::padding_top,     quartz::Dimension{1.5f, quartz::Unit::Percent});
    horizontalGui3.style.Set(quartz::Properties::padding_bottom,  quartz::Dimension{1.5f, quartz::Unit::Percent});
    horizontalGui3.style.Set(quartz::Properties::layout,          quartz::Layouts::horizontal);
    horizontalGui3.style.Set(quartz::Properties::background_color, backgroundColor);

    // Radius
    GuiObjectComponent radiusLabelObject;
    TextMeshComponent labelRadius;
    GuiObjectComponent radiusButtonObject;
    ButtonComponent radiusButton;
    TextMeshComponent radiusButtonLabel;

    {
        // Label
        radiusLabelObject.style.Set(quartz::Properties::width,          quartz::Dimension{50.0f, quartz::Unit::Percent});
        radiusLabelObject.style.Set(quartz::Properties::margin_top,     quartz::Dimension{30u, quartz::Unit::Pixels});
        radiusLabelObject.style.Set(quartz::Properties::padding_top,    quartz::Dimension{10u, quartz::Unit::Pixels});
        radiusLabelObject.style.Set(quartz::Properties::padding_bottom, quartz::Dimension{10u, quartz::Unit::Pixels});
        radiusLabelObject.style.Set(quartz::Properties::font_size,      quartz::Dimension{20.0f*fontSize2, quartz::Unit::Percent});
        radiusLabelObject.style.Set(quartz::Properties::color,          fontColor);
        radiusLabelObject.style.Set(quartz::Properties::background_color, backgroundColor);
        radiusLabelObject.zLayer = -0.4f;

        labelRadius.content = "Radius";

        // Button
        radiusButtonObject.style.Set(quartz::Properties::width,          quartz::Dimension{50.0f, quartz::Unit::Percent});
        radiusButtonObject.style.Set(quartz::Properties::margin_top,     quartz::Dimension{30u, quartz::Unit::Pixels});
        radiusButtonObject.style.Set(quartz::Properties::background_color, glm::vec4(1.0f,1.0f,0.0f,1.0f));
        radiusButtonObject.style.Set(quartz::Properties::color,          glm::vec4(0.0f,1.0f,1.0f,1.0f));
        radiusButtonObject.zLayer = -0.4f;
        
        radiusButtonLabel.content = "30";

        radiusButton.onClick = [this, radiusButtonEntity](){
            auto& particleSettings = mRegistry.GetSingleton<ParticleSettingsComponent>();
            auto& labelPool        = mRegistry.GetComponentPool<TextMeshComponent>();
            auto& label = labelPool.GetComponent(radiusButtonEntity);

            particleSettings.radius = std::stof(label.content);
            std::cout << particleSettings.radius << std::endl;
        };
    }

    // Mass
    GuiObjectComponent massLabelObject;
    TextMeshComponent labelMass;
    GuiObjectComponent massButtonObject;
    ButtonComponent massButton;
    TextMeshComponent massButtonLabel;

    {
        // Label
        massLabelObject.style.Set(quartz::Properties::width,          quartz::Dimension{50.0f, quartz::Unit::Percent});
        massLabelObject.style.Set(quartz::Properties::margin_top,     quartz::Dimension{30u, quartz::Unit::Pixels});
        massLabelObject.style.Set(quartz::Properties::padding_top,    quartz::Dimension{10u, quartz::Unit::Pixels});
        massLabelObject.style.Set(quartz::Properties::padding_bottom, quartz::Dimension{10u, quartz::Unit::Pixels});
        massLabelObject.style.Set(quartz::Properties::font_size,      quartz::Dimension{20.0f*fontSize2, quartz::Unit::Percent});
        massLabelObject.style.Set(quartz::Properties::color,          fontColor);
        massLabelObject.style.Set(quartz::Properties::background_color, backgroundColor);
        massLabelObject.zLayer = -0.4f;

        labelMass.content = "Mass";

        // Button
        massButtonObject.style.Set(quartz::Properties::width,          quartz::Dimension{50.0f, quartz::Unit::Percent});
        massButtonObject.style.Set(quartz::Properties::margin_top,     quartz::Dimension{30u, quartz::Unit::Pixels});
        massButtonObject.style.Set(quartz::Properties::background_color, glm::vec4(1.0f,1.0f,0.0f,1.0f));
        massButtonObject.style.Set(quartz::Properties::color,          glm::vec4(0.0f,1.0f,1.0f,1.0f));
        massButtonObject.zLayer = -0.4f;
        
        massButtonLabel.content = "30";

        massButton.onClick = [this, massButtonEntity](){
            std::cout << "massButton" << std::endl;
        };
    }

    // Velocity
    GuiObjectComponent velocityLabelObject;
    TextMeshComponent labelVelocity;
    ButtonComponent velocityButton;
    TextMeshComponent velocityButtonLabel;
    GuiObjectComponent velocityButtonObject;

    {
        // Label style
        velocityLabelObject.style.Set(quartz::Properties::width,          quartz::Dimension{50.0f, quartz::Unit::Percent});
        velocityLabelObject.style.Set(quartz::Properties::margin_top,     quartz::Dimension{30u, quartz::Unit::Pixels});
        velocityLabelObject.style.Set(quartz::Properties::padding_top,    quartz::Dimension{10u, quartz::Unit::Pixels});
        velocityLabelObject.style.Set(quartz::Properties::padding_bottom, quartz::Dimension{10u, quartz::Unit::Pixels});
        velocityLabelObject.style.Set(quartz::Properties::font_size,      quartz::Dimension{20.0f*fontSize2, quartz::Unit::Percent});
        velocityLabelObject.style.Set(quartz::Properties::color,          fontColor);
        velocityLabelObject.style.Set(quartz::Properties::background_color, backgroundColor);
        velocityLabelObject.zLayer = -0.4f;

        labelVelocity.content = "Velocity";

        // Button style
        velocityButtonObject.style.Set(quartz::Properties::width,          quartz::Dimension{50.0f, quartz::Unit::Percent});
        velocityButtonObject.style.Set(quartz::Properties::margin_top,     quartz::Dimension{30u, quartz::Unit::Pixels});
        velocityButtonObject.style.Set(quartz::Properties::background_color, glm::vec4(1.0f,1.0f,0.0f,1.0f));
        velocityButtonObject.style.Set(quartz::Properties::color,          glm::vec4(0.0f,1.0f,1.0f,1.0f));
        velocityButtonObject.zLayer = -0.4f;

        velocityButtonLabel.content = "300";

        velocityButton.onClick = [this, velocityButtonEntity](){
            std::cout << "velocityButton" << std::endl;
        };
    }


    labelObject.parentID    = backgroundEntity;
    horizontalGui1.parentID = backgroundEntity;
    horizontalGui2.parentID = backgroundEntity;
    horizontalGui3.parentID = backgroundEntity;

    radiusLabelObject.parentID   = horizontalGui1Entity;
    radiusButtonObject.parentID  = horizontalGui1Entity;
    
    massLabelObject.parentID     = horizontalGui2Entity;
    massButtonObject.parentID    = horizontalGui2Entity;

    velocityLabelObject.parentID = horizontalGui3Entity;
    velocityButtonObject.parentID= horizontalGui3Entity;

    mRegistry.EmplaceComponent<GuiObjectComponent>(backgroundEntity, background);
    mRegistry.EmplaceComponent<GuiMeshComponent>(backgroundEntity);

    mRegistry.EmplaceComponent<GuiObjectComponent>(labelEntity, labelObject);
    mRegistry.EmplaceComponent<TextMeshComponent>(labelEntity, label);
    mRegistry.EmplaceComponent<GuiMeshComponent>(labelEntity);

    mRegistry.EmplaceComponent<GuiObjectComponent>(horizontalGui1Entity, horizontalGui1);
    mRegistry.EmplaceComponent<GuiMeshComponent>(horizontalGui1Entity);
    mRegistry.EmplaceComponent<GuiObjectComponent>(horizontalGui2Entity, horizontalGui2);
    mRegistry.EmplaceComponent<GuiMeshComponent>(horizontalGui2Entity);
    mRegistry.EmplaceComponent<GuiObjectComponent>(horizontalGui3Entity, horizontalGui3);
    mRegistry.EmplaceComponent<GuiMeshComponent>(horizontalGui3Entity);

    mRegistry.EmplaceComponent<GuiObjectComponent>(radiusEntity, radiusLabelObject);
    mRegistry.EmplaceComponent<GuiMeshComponent>(radiusEntity);
    mRegistry.EmplaceComponent<TextMeshComponent>(radiusEntity, labelRadius);
    
    mRegistry.EmplaceComponent<GuiObjectComponent>(radiusButtonEntity, radiusButtonObject);
    mRegistry.EmplaceComponent<GuiMeshComponent>(radiusButtonEntity);
    mRegistry.EmplaceComponent<ButtonComponent>(radiusButtonEntity, radiusButton);
    mRegistry.EmplaceComponent<TextMeshComponent>(radiusButtonEntity, radiusButtonLabel);
    
    mRegistry.EmplaceComponent<GuiObjectComponent>(massEntity, massLabelObject);
    mRegistry.EmplaceComponent<GuiMeshComponent>(massEntity);
    mRegistry.EmplaceComponent<TextMeshComponent>(massEntity, labelMass);
    
    mRegistry.EmplaceComponent<GuiObjectComponent>(massButtonEntity, massButtonObject);
    mRegistry.EmplaceComponent<GuiMeshComponent>(massButtonEntity);
    mRegistry.EmplaceComponent<ButtonComponent>(massButtonEntity, massButton);
    mRegistry.EmplaceComponent<TextMeshComponent>(massButtonEntity, massButtonLabel);
    
    mRegistry.EmplaceComponent<GuiObjectComponent>(velocityEntity, velocityLabelObject);
    mRegistry.EmplaceComponent<GuiMeshComponent>(velocityEntity);
    mRegistry.EmplaceComponent<TextMeshComponent>(velocityEntity, labelVelocity);

    mRegistry.EmplaceComponent<GuiObjectComponent>(velocityButtonEntity, velocityButtonObject);
    mRegistry.EmplaceComponent<GuiMeshComponent>(velocityButtonEntity);
    mRegistry.EmplaceComponent<ButtonComponent>(velocityButtonEntity, velocityButton);
    mRegistry.EmplaceComponent<TextMeshComponent>(velocityButtonEntity, velocityButtonLabel);

    quartz::StyleSetupSystem styleSystem(mFontManager, mWindowData.mScreenWidth, mWindowData.mScreenHeight);
    quartz::GuiVertexSetupSystem guiVertexSystem;
    styleSystem.Update(mRegistry);
    guiVertexSystem.Update(mRegistry);
}