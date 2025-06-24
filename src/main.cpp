// C++ standard libraries
#include <iostream>
#include <string>

// Third_party libraries
#include <SDL3/SDL.h>

// Own libraries
#include "sapphire/application/fluid_app.hpp"

int main(int argc, char* argv[]) {
    FluidApp app("./config/config.json");
    app.Run();

    return 0;
}