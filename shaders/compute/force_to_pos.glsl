#version 450 core
layout(local_size_x = 64) in;

layout(std430, binding = 0) buffer sphereComponent      { vec4 positionAndRadius[]; };
layout(std430, binding = 1) buffer velocityComponent    { vec3 velocity[];          };
layout(std430, binding = 2) buffer forceComponent       { vec3 force[];             };
layout(std430, binding = 3) buffer massComponent        { float mass[];             };

layout(std430, binding = 4) buffer sphereComponentLoc   { uint sphereIDs[];         };
layout(std430, binding = 5) buffer velocityComponentLoc { uint velocityIDs[];       };
layout(std430, binding = 6) buffer forceComponentLoc    { uint forceIDs[];          };
layout(std430, binding = 7) buffer massComponentLoc     { uint massIDs[];           };

layout(std430, binding = 8) buffer denseSphereIDs       { uint denseIDs[];          };

uniform float uTimeStep;

void main() {
    uint currentID = gl_GlobalInvocationID.x;
    if(currentID >= denseIDs.length()) {
        return;
    }

    uint currentPointID = denseIDs[currentID];

    velocity[velocityIDs[currentPointID]] += (force[forceIDs[currentPointID]] / mass[massIDs[currentPointID]]) * uTimeStep;
    positionAndRadius[sphereIDs[currentPointID]] += vec4(velocity[velocityIDs[currentPointID]], 0.0f) * uTimeStep;
}