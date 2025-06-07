#version 450 core

// Input Buffers
// Data
layout(std430, binding = 0) buffer positionAndRadius {
    vec4 positionAndRadius[];
};
layout(std430, binding = 1) buffer spatialHash {
    uint chil
}
layout(std430, binding = 2) buffer spatialHashPosition {

}

// Uniforms
uniform float uRadius;
uniform int   uSpatialLength;
uniform float uSpatialLengthMax;

void main() {

}