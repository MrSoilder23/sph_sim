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
    std::vector<glm::vec4> colors;

    std::string content;
    unsigned int numIndices = 0;

    const quartz::FontAtlas* fontAtlas = nullptr;
};