#pragma once
// Third_party libraries
#include <glm/ext/vector_float2.hpp>

struct AABB {
    // Corners of AABB
    glm::vec2 min;
    glm::vec2 max;
};