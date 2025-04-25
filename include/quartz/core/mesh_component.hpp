#pragma once
// Third_party libraries
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

// Own libraries
#include "./quartz/core/component.hpp"

struct MeshComponent : public IComponent {
    glm::vec3 position = glm::vec3(0.0f);;
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 scale    = glm::vec3(0.0f);;

    glm::mat4 transform;

    bool isDirty = true;
};