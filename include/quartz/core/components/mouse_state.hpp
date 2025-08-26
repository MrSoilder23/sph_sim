#pragma once
// C++ standard libraries
#include <vector>

// Third_party library
#include <glad/glad.h>
#include <glm/glm.hpp>

struct MouseStateComponent {
    glm::vec2 position = {0.0f, 0.0f};
    glm::vec2 delta    = {0.0f, 0.0f};

    bool leftPressed  = false;
    bool leftClicked  = false;
    bool rightPressed = false;
    bool rightClicked = false;

    float scroll = 0.0f;
};