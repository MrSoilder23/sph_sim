#version 450 core
layout(local_size_x = 64) in;

// Dense components array
layout(std430, binding = 0) buffer sphereComponent      { vec4  positionAndRadius[]; };
layout(std430, binding = 1) buffer pressureComponent    { float pressure[];          };
layout(std430, binding = 2) buffer densityComponent     { float densities[];         };
layout(std430, binding = 3) buffer massComponent        { float mass[];              };

// Component locations
layout(std430, binding = 4) buffer sphereComponentLoc   { uint sphereIDs[];          };
layout(std430, binding = 5) buffer pressureComponentLoc { uint pressureIDs[];        };
layout(std430, binding = 6) buffer densityComponentLoc  { uint densityIDs[];         };
layout(std430, binding = 7) buffer massComponentLoc     { uint massIDs[];            };

layout(std430, binding = 8) buffer denseSphereIDs       { uint denseIDs[];           };

// SpatialHash
layout(std430, binding = 9) buffer hashTable            { uint bucketHeads[];       };
layout(std430, binding = 10) buffer nextPointers        { uint next[];              };
layout(std430, binding = 11) buffer bucketKeys          { uint keys[];              };

// Uniforms
uniform float uStiffness;
uniform float uRestDensity;
uniform float uSmoothingLength;

uniform float uCellSize;
uniform uint uHashSize;


// Helper functions
float CubicSplineKernel(float radius) {
    const float PI = 3.14159265358979323846f;

    float q = radius / uSmoothingLength; // normalizedDistance
    if (q < 0) return 0.0f;
    if (q >= 2.0f) return 0.0f;

    const float normalization = 1.0f / (PI * uSmoothingLength * uSmoothingLength * uSmoothingLength);
    if (q <= 1.0f) {
        return normalization * (1.0f - 1.5f*q*q + 0.75f*q*q*q);
    }
    
    float t = 2.0f - q;
    return normalization * 0.25f * t * t * t;
}

uint HashFunction(ivec3 gridCell) {
    const uint p1 = 73856093, p2 = 19349663, p3 = 83492791;
    return (gridCell.x * p1 ^ gridCell.y * p2 ^ gridCell.z * p3) & (uHashSize - 1);
}

float ComputeDensity(uint currentSphereID) {
    float density = 0.0f;

    vec3 pointPos = positionAndRadius[sphereIDs[currentSphereID]].xyz;
    ivec3 centerCell = ivec3(floor(pointPos / uCellSize));

    // Find neighbors directly because I don't have enough memory on the gpu
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            for(int z = -1; z <= 1; z++) {
                ivec3 neighborCell = centerCell + ivec3(x, y, z);
                uint targetBucketKey = HashFunction(neighborCell);

                if(targetBucketKey >= bucketHeads.length()) {
                    continue;
                }

                uint neighborID = bucketHeads[targetBucketKey];

                while(neighborID != 0xFFFFFFFF) {
                    if(keys[neighborID] == targetBucketKey) {
                        vec3 dist = pointPos - positionAndRadius[sphereIDs[neighborID]].xyz;
                        float radius = length(dist);

                        if(radius <= uSmoothingLength) {
                            density += mass[massIDs[neighborID]] * CubicSplineKernel(radius);
                        }
                    }
                    neighborID = next[neighborID];
                }
            }
        }
    }

    return max(density, 1e-5f);
}

float ComputePressure(float density) {
    return uStiffness * (density - uRestDensity);
}

void main() {
    uint currentID = gl_GlobalInvocationID.x;
    if(currentID >= denseIDs.length()) {
        return;
    }

    uint currentPointID = denseIDs[currentID];

    float density = ComputeDensity(currentPointID);

    densities[densityIDs[currentPointID]] = density;
    pressure[pressureIDs[currentPointID]] = ComputePressure(density);
}