#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colors;

uniform mat4 uProjectionMatrix;

out vec4 vColor;

void main() {
    vColor = colors;

    gl_Position = uProjectionMatrix * vec4(position, 1.0f);
}