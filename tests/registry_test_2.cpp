// C++ standard libraries
#include <iostream>

// Own libraries
#include "bismuth/registry.hpp"

struct PositionComponent{
    int x;
    int y;
};

struct other{
    int a;
    int b;
};

int main() {
    bismuth::Registry registry;

    for(int i = 0; i < 1'000'000; i++) {
        auto entityID = registry.CreateEntity();
        registry.EmplaceComponent<PositionComponent>(entityID, 0,2);
        registry.EmplaceComponent<other>(entityID, 0,2);
    }
}