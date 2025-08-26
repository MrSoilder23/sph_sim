#pragma once
// Third_party libraries
#include <glm/glm.hpp>

struct GuiCameraComponent {
    int width;
    int height;

    glm::mat4 projectionMatrix = glm::mat4(1.0f);

    bool isDirty = true;
};