#pragma once
// Third_party libraries
#include <glm/glm.hpp>

struct CameraComponent {
    float fov = 45.0f;
    float aspectRatio = 16.0f/9.0f;
    float near = 0.1f;
    float far = 100.0f;

    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix       = glm::mat4(1.0f);
    glm::mat4 viewProjection;

    bool isDirty = true;
};