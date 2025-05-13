#pragma once
// Third_party library
#include <glad/glad.h>

struct MeshComponent {
    GLuint VAO;
    GLuint VBO;

    GLuint shaderProgram;

    float radius;
};