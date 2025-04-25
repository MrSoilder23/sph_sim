// C++ standard libraries
#include <iostream>

// Third_party libraries
#include <SDL3/SDL.h>

// Own libraries
#include "quartz/engine.hpp"

quartz::Engine gEngine;

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

}

void Loop(float deltaTime) {

}

int main(int argc, char* argv[]) {

    gEngine.Initialize("./config/config.json");

    gEngine.SetEventCallback(Event);
    gEngine.SetSystemCallback(System);
    gEngine.SetUpdateCallback(Loop);

    gEngine.Run();
    gEngine.Shutdown();

    return 0;
}