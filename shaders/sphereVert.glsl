#version 450 core

layout(location = 0) in vec4 instanceData;

uniform mat4 uProjectionMatrix;
uniform vec3 uCameraPosition;

void main() {
    float dist = length(instanceData.xyz - uCameraPosition);

    gl_PointSize = instanceData.w * 100.0f / dist;

    gl_Position = uProjectionMatrix * vec4(instanceData.xyz, 1.0f);
}