#include "./quartz/core/systems/camera_system.hpp"

void quartz::CameraSystem::Update(bismuth::Registry& registry) {
    auto cameraView = registry.GetView<CameraComponent, TransformComponent>();

    for(auto [entity, camera, transform] : cameraView) {
        if(camera.isDirty) {
            camera.projectionMatrix = glm::perspective(
                camera.fov, 
                camera.aspectRatio, 
                camera.near, 
                camera.far
            );
            camera.isDirty = false;
        }
        
        glm::vec3 forward = transform.rotation * glm::vec3(0.0f, 0.0f, -1.0f); 
        glm::vec3 up = transform.rotation * glm::vec3(0.0f, 1.0f, 0.0f);

        glm::mat4 view = glm::lookAt(
            transform.position,
            transform.position + forward,
            up
        );

        camera.viewProjection = camera.projectionMatrix * view;
    }
}