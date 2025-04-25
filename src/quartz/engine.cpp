#include "./quartz/engine.hpp"
#include "glad/glad.h"
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

quartz::Engine::~Engine() {
    Shutdown();
}

void quartz::Engine::Initialize(const std::string& configPath) {
    // Reading Json config file
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
    mScreenWidth = config["window"]["width"];
    mScreenHeight = config["window"]["height"];
    bool fullscreen = config["window"]["fullscreen"];
    bool vsync = config["rendering"]["vsync"];
    
    // Flags
    Uint32 windowFlags = SDL_WINDOW_OPENGL;
    if(fullscreen) {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    mWindow = SDL_CreateWindow(
        title.data(),
        mScreenWidth,
        mScreenHeight,
        windowFlags
    );

    if (!mWindow) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    mOpenGLContext = SDL_GL_CreateContext(mWindow);

    if(mOpenGLContext == nullptr) {
        std::cerr << "SDL opengl context was not able to initialize" << std::endl;
        exit(1);
    }

    if(!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        std::cerr << "Glad was not initialized" << std::endl;
        exit(1);
    }

    if(!vsync) {
        SDL_GL_SetSwapInterval(0);
    }

}

void quartz::Engine::Run() {
    
    while (!mQuit) {
        Uint64 now = SDL_GetPerformanceCounter();
        double deltaTime = (now - mLastTime) / double(SDL_GetPerformanceFrequency());
        mLastTime = now;
        
        mEventCallback(deltaTime);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        glViewport(0,0, mScreenWidth, mScreenHeight);
        glClearColor(0.1f, 0.4f, 0.2f, 1.0f);

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

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