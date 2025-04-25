#include "./quartz/graphics/instanced_renderer_system.hpp"
#include "quartz/core/instance_component.hpp"
#include "quartz/core/mesh_component.hpp"
#include "quartz/core/transform_component.hpp"

void quartz::InstancedRendererSystem::Update(bismuth::Registry& registry) {
    const auto cameraView = registry.GetView<CameraComponent>();
    auto modelView = registry.GetView<InstanceComponent, MeshComponent, TransformComponent>();
    
    for(auto [_, camera] : cameraView) {
        for(auto [_, _, mesh, transform] : modelView) {
            
        }
    }
}