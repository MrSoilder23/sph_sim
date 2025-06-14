#pragma once
// Third_party libraries
#include <glm/glm.hpp>

struct ForceComponent {
    glm::vec4 f; // Vec4 for padding in gpu
};