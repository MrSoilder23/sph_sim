#pragma once
// C++ standard libraries
#include <vector>

// Third_party library
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

struct GuiMeshComponent {
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
    GLuint colorVBO = 0;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec4> colors;
};