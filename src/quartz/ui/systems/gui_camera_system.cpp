#include "quartz/ui/systems/gui_camera_system.hpp"

void quartz::GuiCameraSystem::Update(bismuth::Registry& registry) {
    auto& guiCameraPool = registry.GetComponentPool<GuiCameraComponent>();

    auto cameraBegin = guiCameraPool.ComponentBegin();
    auto cameraEnd   = guiCameraPool.ComponentEnd();

    for(auto guiCamera = cameraBegin; guiCamera != cameraEnd; ++guiCamera) {
        if(guiCamera->isDirty) {
            guiCamera->projectionMatrix = glm::ortho(0.0f, (float)guiCamera->width, (float)guiCamera->height, 0.0f, -1.0f, 1.0f);
            guiCamera->isDirty = false;
        }
    }
}