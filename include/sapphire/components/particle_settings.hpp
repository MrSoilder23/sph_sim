#pragma once
// Third_party libraries
#include <glm/glm.hpp>

struct ParticleSettingsComponent {
    float radius = 3.0f;
    float mass   = 0.3f;
    glm::vec4 velocity = glm::vec4(0.0f);
};