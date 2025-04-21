// C++ standard libraries
#include <iostream>

// Own libraries
#include "./storage/component_pool.hpp"

int main() {
    ComponentPool<int> intPool;

    int someComponent = 3;
    intPool.AddComponent(0, someComponent);
    intPool.AddComponent(2, someComponent);
    intPool.AddComponent(6, someComponent);
    
    auto& entityDenseArray = intPool.GetDenseEntities();

    std::cout << "Entities: ";
    for(auto& entityID : entityDenseArray) {
        std::cout << entityID << " ";
    }

    std::cout << std::endl;
    int i = 2;
    for(auto it = intPool.ComponentBegin(); it != intPool.ComponentEnd(); ++it) {
        *it += i++;
    }
    for(auto it = intPool.ComponentBegin(); it != intPool.ComponentEnd(); ++it) {
        std::cout << *it << std::endl;
    }

    std::cout << "FINISHED" << std::endl;
}