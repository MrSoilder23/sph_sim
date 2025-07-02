#version 450 core

layout(location = 0) position;

uniform mat4 uProjectionMatrix;

void main() {
    gl_PointSize = uProjectionMatrix * vec4(position, 1.0f);
}