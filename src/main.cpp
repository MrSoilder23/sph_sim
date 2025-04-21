// C++ standard libraries
#include <iostream>

// Own libraries
#include "./registry.hpp"

int main() {
    Registry registry;

    std::cout << "int: "   << registry.GetPoolID<int>() << std::endl;
    std::cout << "float: " << registry.GetPoolID<float>() << std::endl;
    std::cout << "int: "   << registry.GetPoolID<int>() << std::endl;
    std::cout << "float: " << registry.GetPoolID<float>() << std::endl;
    std::cout << "double: "<< registry.GetPoolID<double>() << std::endl;
}