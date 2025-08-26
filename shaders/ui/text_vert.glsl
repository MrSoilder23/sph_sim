#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec4 colors;

uniform mat4 uProjectionMatrix;
uniform sampler2D uFontAtlas;

out vec2 vTexCoords;
out vec4 vColor;

void main() {
    vTexCoords = texCoords;
    vColor = colors;

    gl_Position = uProjectionMatrix * vec4(position, 1.0f);
}