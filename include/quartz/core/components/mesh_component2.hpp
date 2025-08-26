#pragma once
// C++ standard libraries
#include <vector>

// Third_party library
#include <glad/glad.h>
#include <glm/glm.hpp>

struct MeshComponent2 {
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    GLuint shaderProgram;

    std::vector<glm::vec3> vertices;
    std::vector<GLuint>    indexes;
};