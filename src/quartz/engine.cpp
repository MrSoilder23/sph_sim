#include "./quartz/engine.hpp"
#include "SDL3/SDL_video.h"

void quartz::Engine::Initialize(const std::string& configPath) {
    std::ifstream configFile(configPath);
    if (!configFile) {
        std::cerr << "Failed to open configuration file." << std::endl;
        exit(1);
    }

    nlohmann::json config;
    configFile >> config;
    
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    std::string title = config["window"]["title"];
    int width = config["window"]["width"];
    int height = config["window"]["height"];
    bool fullscreen = config["window"]["fullscreen"];
    
    Uint32 windowFlags = SDL_WINDOW_OPENGL;
    if(fullscreen) {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    }

    SDL_Window* window = SDL_CreateWindow(
        title.data(),
        width,
        height,
        windowFlags
    );

    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

}

void quartz::Engine::Run() {
    
    while (!mQuit) {
        float deltaTime = 1;
        mEventCallback(deltaTime);

        // glEnable(GL_DEPTH_TEST);
        // glEnable(GL_CULL_FACE);

        // glViewport(0,0, mScreenWidth, mScreenHeight);
        // glClearColor(r, g, b, 1.0f);

        // glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        mUpdateCallback(deltaTime);

        mSystemCallback(deltaTime);

        SDL_GL_SwapWindow(mWindow);

    }
}

void quartz::Engine::Shutdown() {
    SDL_DestroyWindow(mWindow);
    mWindow = nullptr;

    SDL_Quit();
}

void quartz::Engine::Quit() {
    mQuit = true;
}

void quartz::Engine::SetEventCallback(const std::function<void(float)>& func) {
    mEventCallback = func;
}
void quartz::Engine::SetUpdateCallback(const std::function<void(float)>& func) {
    mUpdateCallback = func;
}
void quartz::Engine::SetSystemCallback(const std::function<void(float)>& func) {
    mSystemCallback = func;
}