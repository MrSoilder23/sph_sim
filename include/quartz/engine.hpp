#pragma once
// C++ standard libraries
#include <functional>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>

// Third_party libraries
#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>

namespace quartz {

class Engine {
    public:
        void Initialize(const std::string& configPath);
        void Run();
        void Shutdown();

        void Quit();

        void SetEventCallback(const std::function<void(float)>& func);
        void SetUpdateCallback(const std::function<void(float)>& func);
        void SetSystemCallback(const std::function<void(float)>& func);
    private:
        SDL_Window* mWindow = nullptr;

        std::function<void(float)> mEventCallback;
        std::function<void(float)> mUpdateCallback;
        std::function<void(float)> mSystemCallback;

        bool mQuit = false;
};

}