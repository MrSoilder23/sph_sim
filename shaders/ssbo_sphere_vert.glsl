#version 450 core

layout(std430, binding = 0) buffer sphereComponent    { vec4 positionAndRadius[]; };
layout(std430, binding = 1) buffer sphereComponentLoc { uint sphereIDs[];    };
layout(std430, binding = 2) buffer denseSphereIDs     { uint denseIDs[];     };

uniform mat4 uProjectionMatrix;
uniform vec3 uCameraPosition;

void main() {
    uint currentPointID = denseIDs[gl_VertexID];
    vec4 currentPoint = positionAndRadius[sphereIDs[currentPointID]];

    float dist = length(currentPoint.xyz - uCameraPosition);

    gl_PointSize = currentPoint.w * 100.0f / dist;
    gl_Position = uProjectionMatrix * vec4(currentPoint.xyz, 1.0f);
}