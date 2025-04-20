// C++ standard libraries
#include <iostream>

// Own libraries
#include "./storage/component_pool.hpp"

int main() {
    ComponentPool<int> intPool;

    intPool.Reserve(100'000'000);

    for(int i = 0; i < 100'000'000; i++) {
        int a = 5;
        intPool.AddComponent(i, a);
    }

    for(int i = 0; i < 100'000'000; i++) {
        int& a = intPool.GetComponent(i);
        a += i;
    }

    for(int i = 0; i < 100'000'000; i++) {
        intPool.RemoveComponent(i);
    }

    std::cout << "FINISHED" << std::endl;
}