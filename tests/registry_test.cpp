// C++ standard libraries
#include <iostream>

// Own libraries
#include "./registry.hpp"

int main() {
    Registry registry;

    const size_t entity = registry.CreateEntity();
    const size_t entity2 = registry.CreateEntity();
    const size_t entity3 = registry.CreateEntity();
    int i = 5;

    registry.EmplaceComponent<int>(entity, i);
    registry.EmplaceComponent<int>(entity2, i);

    std::cout << registry.HasComponent<int>(entity) << std::endl;
    std::cout << registry.HasComponent<float>(entity) << std::endl;
    std::cout << registry.HasComponent<int>(entity3) << std::endl;
}