// C++ standard libraries
#include <iostream>

// Own libraries
#include "./registry.hpp"

struct PositionComponent {
    int x;
    int y;
};
struct OtherComponent {
    int x;
    int y;
};

int main() {
    Registry registry;

    auto entity0 = registry.CreateEntity();
    auto entity1 = registry.CreateEntity();
    auto entity2 = registry.CreateEntity();
    auto entity3 = registry.CreateEntity();
    auto entity4 = registry.CreateEntity();

    registry.EmplaceComponent<OtherComponent>(entity0, 1,2);
    registry.EmplaceComponent<OtherComponent>(entity2, 1,2);
    registry.EmplaceComponent<OtherComponent>(entity3, 1,2);

    registry.EmplaceComponent<PositionComponent>(entity0, 1,2);
    registry.EmplaceComponent<PositionComponent>(entity1, 1,2);
    registry.EmplaceComponent<PositionComponent>(entity2, 1,2);
    registry.EmplaceComponent<PositionComponent>(entity3, 1,2);

    auto view = registry.GetView<PositionComponent, OtherComponent>();

    for(auto [entity, a, b] : view) {
        std::cout << "entity: " << entity << " component1: " << a.x << ":" << a.y << " component2: " << b.x << ":" << b.y << std::endl;
    }
    
    registry.RemoveEntity(entity2);
    std::cout << std::endl;

    for(auto [entity, a, b] : view) {
        std::cout << "entity: " << entity << " component1: " << a.x << ":" << a.y << " component2: " << b.x << ":" << b.y << std::endl;
    }
}