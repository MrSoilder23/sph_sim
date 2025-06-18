#version 450 core
layout(local_size_x = 64) in;

layout(std430, binding = 0) buffer sphereComponent    { vec4 positionAndRadius[]; };
layout(std430, binding = 1) buffer hashTable          { uint bucketHeads[];       };
layout(std430, binding = 2) buffer nextPointers       { uint next[];              };
layout(std430, binding = 3) buffer bucketKeys         { uint keys[];              };

layout(std430, binding = 4) buffer sphereComponentLoc { uint sphereIDs[];         };

layout(std430, binding = 5) buffer denseSphereIDs     { uint denseIDs[];          };

// Uniforms
uniform float uCellSize;
uniform uint uHashSize;


uint HashFunction(ivec3 gridCell) {
    const uint p1 = 73856093, p2 = 19349663, p3 = 83492791;
    return (gridCell.x * p1 ^ gridCell.y * p2 ^ gridCell.z * p3) & (uHashSize - 1);
}

void main() {
    uint currentID = gl_GlobalInvocationID.x;
    if(currentID >= denseIDs.length()) {
        return;
    }

    uint currentPointID = denseIDs[currentID];

    vec3 position = positionAndRadius[sphereIDs[currentPointID]].xyz;
    ivec3 gridCell = ivec3(floor(position / uCellSize));
    uint bucketKey = HashFunction(gridCell);

    uint previousHead = atomicExchange(bucketHeads[bucketKey], currentPointID);

    next[currentPointID] = previousHead;
    keys[currentPointID] = bucketKey;
}