#pragma once
// C++ standard libraries
#include <SDL3/SDL_video.h>
#include <functional>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>

// Third_party libraries
#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>
#include <glad/glad.h>

namespace quartz {

class Engine {
    public:
        ~Engine();

        void Initialize(const std::string& configPath);
        void Run();
        void Shutdown();

        void Quit();

        void SetEventCallback(const std::function<void(float)>& func);
        void SetUpdateCallback(const std::function<void(float)>& func);
        void SetSystemCallback(const std::function<void(float)>& func);
    private:
        SDL_Window* mWindow = nullptr;
        SDL_GLContext mOpenGLContext;

        std::function<void(float)> mEventCallback;
        std::function<void(float)> mUpdateCallback;
        std::function<void(float)> mSystemCallback;

        int mScreenWidth;
        int mScreenHeight;

        Uint64 mLastTime = 0;

        bool mQuit = false;
};

}