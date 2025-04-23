// C++ standard libraries
#include <iostream>

// Own libraries
#include "./bismuth/registry.hpp"

struct PositionComponent {
    int x;
    int y;
};

int main() {
    bismuth::Registry registry;

    const size_t entity = registry.CreateEntity();
    const size_t entity2 = registry.CreateEntity();
    const size_t entity3 = registry.CreateEntity();
    int i = 5;

    registry.EmplaceComponent<PositionComponent>(entity, 4, 2);
    registry.EmplaceComponent<int>(entity2, i);

    std::cout << registry.HasComponent<PositionComponent>(entity)   << std::endl;
    std::cout << registry.HasComponent<float>(entity) << std::endl;
    std::cout << registry.HasComponent<int>(entity3)  << std::endl;
}