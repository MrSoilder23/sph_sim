#include "./sapphire/test.hpp"

void TestSystem::Update(bismuth::Registry& registry) {
    auto sphereView = registry.GetView<InstanceComponent, SphereComponent>();

    // To-Do change after bismuth iterator is improved
    // This for loop is 60% faster than provided from my class
    const auto viewEntities = sphereView.GetSmallestDense();
    auto& instancePool = registry.GetComponentPool<InstanceComponent>();
    auto& spherePool = registry.GetComponentPool<SphereComponent>();

    #pragma omp parallel for
    for(int i = 0; i < sphereView.SizeHint(); i++) {
        size_t entityID = (*viewEntities)[i];
        if(!instancePool.HasComponent(entityID) && !spherePool.HasComponent(entityID)) {
            continue;
        }    

        spherePool.GetComponent(entityID).positionAndRadius.y -= 0.1f;
    }

    // for(const auto& [entity, instance, sphere] : spheres) {
    //     // if(sphere.positionAndRadius.z <= -2.0f) {
    //     //     sphere.positionAndRadius.z += 0.1f;
    //     // } else if(sphere.positionAndRadius.z >= -15.0f) {
    //     //     sphere.positionAndRadius.z -= 0.1f;
    //     // }
    //     sphere.positionAndRadius.y -= 0.1f;
    // }   
}