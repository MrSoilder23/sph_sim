// Own libraries
#include "sapphire/application/fluid_app.hpp"

int main(int argc, char* argv[]) {
    FluidApp app("./config/config.json");
    app.Run();

    return 0;
}