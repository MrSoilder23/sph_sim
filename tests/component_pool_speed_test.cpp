// C++ standard libraries
#include <iostream>

// Own libraries
#include "./bismuth/storage/component_pool.hpp"

int main() {
    bismuth::ComponentPool<int> intPool;

    int amountOfEntities = 100'000'000;

    intPool.Reserve(amountOfEntities);

    for(int i = 0; i < amountOfEntities; i++) {
        int a = 5;
        intPool.AddComponent(i, a);
    }

    int i = 0;
    auto it = intPool.ComponentEnd();
    auto itEnd = intPool.ComponentEnd();
    for(; it != itEnd; ++it) {
        *it += i++;
    }

    for(int i = 0; i < amountOfEntities; i++) {
        intPool.RemoveComponent(i);
    }

    std::cout << "FINISHED" << std::endl;
}