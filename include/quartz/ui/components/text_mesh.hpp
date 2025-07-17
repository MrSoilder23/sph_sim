#pragma once
// C++ standard libraries
#include <vector>
#include <string>

// Third_party libraries
#include <glad/glad.h>
#include <glm/glm.hpp>

// Own libraries
#include "quartz/ui/utils/font_manager.hpp"

struct TextMeshComponent {
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
    GLuint VBOcolor = 0;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uv;

    std::string content;
    glm::vec4 color = glm::vec4(1.0f);
    unsigned int numIndices = 0;

    const quartz::FontAtlas* fontAtlas = nullptr;
};