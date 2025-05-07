#include "./sapphire/test.hpp"

void TestSystem::Update(bismuth::Registry& registry) {
    auto spheres = registry.GetView<InstanceComponent, SphereComponent>();

    for(auto [entity, instance, sphere] : spheres) {
        // if(sphere.positionAndRadius.z <= -2.0f) {
        //     sphere.positionAndRadius.z += 0.1f;
        // } else if(sphere.positionAndRadius.z >= -15.0f) {
        //     sphere.positionAndRadius.z -= 0.1f;
        // }
        sphere.positionAndRadius.y -= 0.1f;
    }   
}