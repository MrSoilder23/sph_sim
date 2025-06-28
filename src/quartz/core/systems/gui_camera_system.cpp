#include "quartz/core/systems/gui_camera_system.hpp"

void quartz::GuiCameraSystem::Update(bismuth::Registry& registry) {
    auto& guiCameraPool = registry.GetComponentPool<GuiCameraComponent>();

    auto& cameraIDs = guiCameraPool.GetDenseEntities();
    for(int i = 0; i < cameraIDs.size(); i++) {
        size_t entityID = cameraIDs[i];
        auto& guiCamera = guiCameraPool.GetComponent(entityID);

        if(guiCamera.isDirty) {
            guiCamera.projectionMatrix = glm::ortho(0.0f, (float)guiCamera.width, (float)guiCamera.height, 0.0f, -1.0f, 1.0f);
            guiCamera.isDirty = false;
        }
    }
}