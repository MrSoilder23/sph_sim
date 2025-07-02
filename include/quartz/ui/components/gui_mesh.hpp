#pragma once
// C++ standard libraries
#include <vector>

// Third_party library
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

struct GuiMeshComponent {
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    std::vector<glm::vec3> vertices;
};