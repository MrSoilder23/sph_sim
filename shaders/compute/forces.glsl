#version 450 core
layout(local_size_x = 64) in;

// Dense components array
layout(std430, binding = 0) buffer sphereComponent      { vec4 positionAndRadius[]; };
layout(std430, binding = 1) buffer velocityComponent    { vec4 velocity[];          };
layout(std430, binding = 2) buffer pressureComponent    { float pressure[];         };
layout(std430, binding = 3) buffer densityComponent     { float densities[];        };
layout(std430, binding = 4) buffer massComponent        { float mass[];             };
layout(std430, binding = 5) buffer forceComponent       { vec4  force[];            };

// Component locations
layout(std430, binding = 6) buffer sphereComponentLoc   { uint sphereIDs[];         };
layout(std430, binding = 7) buffer velocityComponentLoc { uint velocityIDs[];       };
layout(std430, binding = 8) buffer pressureComponentLoc { uint pressureIDs[];       };
layout(std430, binding = 9) buffer densityComponentLoc  { uint densityIDs[];        };
layout(std430, binding = 10) buffer massComponentLoc    { uint massIDs[];           };
layout(std430, binding = 11) buffer forceComponentLoc   { uint forceIDs[];          };

layout(std430, binding = 12) buffer denseSphereIDs      { uint denseIDs[];          };

// SpatialHash
layout(std430, binding = 13) buffer hashTable           { uint bucketHeads[];       };
layout(std430, binding = 14) buffer nextPointers        { uint next[];              };
layout(std430, binding = 15) buffer bucketKeys          { uint keys[];              };

// Uniforms
uniform float uSmoothingLength;
uniform float uG;

uniform float uCellSize;
uniform uint uHashSize;


// Helper functions
float CubicSplineDerivative(float radius) {
    const float PI = 3.14159265358979323846f;

    float normalizedDistance = radius / uSmoothingLength;
    if(normalizedDistance <= 0.0f || normalizedDistance >= 2.0f) {
        return 0.0f;
    }

    const float normalization = 1.0f / (PI * uSmoothingLength * uSmoothingLength * uSmoothingLength);
    
    if(normalizedDistance <= 1.0f) {
        return normalization * (-3.0f * normalizedDistance + 2.25f * normalizedDistance * normalizedDistance) / uSmoothingLength;
    } else {
        float factor = 2.0f - normalizedDistance;
        return -normalization * 0.75f * factor * factor / uSmoothingLength;
    }
}

vec3 CubicSplineGradient(vec3 deltaPoint, float radius) {
    if(radius == 0.0f) {
        return vec3(0.0f);
    }

    float dwdr = CubicSplineDerivative(radius);
    return dwdr * (deltaPoint / radius);
}

float CubicSplineLaplacian(float radius) {
    const float PI = 3.14159265358979323846f;

    float normalizedDistance = radius / uSmoothingLength;
    if(normalizedDistance >= 2.0f) {
        return 0.0f;
    }
    if(normalizedDistance == 0.0f) {
        return 0.0f;
    }

    const float normalization = 45.0f / (PI * uSmoothingLength * uSmoothingLength * uSmoothingLength * uSmoothingLength * uSmoothingLength);
    if(normalizedDistance <= 1.0f) {
        return normalization * (1.0f - normalizedDistance);
    } else {
        return normalization * (2.0f - normalizedDistance) * (2.0f - normalizedDistance) * (2.0f - normalizedDistance);
    }
}

uint HashFunction(ivec3 gridCell) {
    const uint p1 = 73856093, p2 = 19349663, p3 = 83492791;
    return (gridCell.x * p1 ^ gridCell.y * p2 ^ gridCell.z * p3) & (uHashSize - 1);
}

// Main Calculations
vec3 ComputeForce(uint currentID) {
    float softening = uSmoothingLength * 0.01f;
    float softeningSquared = softening*softening;

    vec3 pressureForce  = vec3(0.0f);
    vec3 viscosityForce = vec3(0.0f);
    vec3 gravityForce   = vec3(0.0f);

    // Current point data
    vec3 currentPointPosition  = positionAndRadius[sphereIDs[currentID]].xyz;
    vec3 currentPointVelocity  = velocity[velocityIDs[currentID]].xyz;
    float currentPointPressure = pressure[pressureIDs[currentID]];
    float currentPointDensity  = densities[densityIDs[currentID]];

    ivec3 centerCell = ivec3(floor(currentPointPosition / uCellSize));

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
                        vec3 dist = currentPointPosition - positionAndRadius[sphereIDs[neighborID]].xyz;
                        float radiusSquared = dot(dist, dist);
                        float radius = sqrt(radiusSquared);

                        if(radius > 0.0f && radius < uSmoothingLength) {
                            float neighborDensity  = densities[densityIDs[neighborID]];
                            float neighborPressure = pressure[pressureIDs[neighborID]];
                            float neighborMass     = mass[massIDs[neighborID]];
                            vec3 neighborVelocity  = velocity[velocityIDs[neighborID]].xyz;

                            // Pressure
                            float pressureTerm = (currentPointPressure / (currentPointDensity*currentPointDensity)) +
                                (neighborPressure / (neighborDensity*neighborDensity));
                            pressureForce += -neighborMass * pressureTerm * CubicSplineGradient(dist, radius);

                            // Viscosity
                            viscosityForce += neighborMass * (neighborDensity * (neighborVelocity - currentPointVelocity)) * CubicSplineLaplacian(radius);

                            // Gravity
                            float distSoft = radiusSquared + softeningSquared;
                            float denominator = sqrt(distSoft*distSoft*distSoft);
                            gravityForce += uG * neighborMass * dist / denominator;
                        }
                    }
                    neighborID = next[neighborID];
                }
            }
        }
    }

    return pressureForce + viscosityForce + gravityForce;
}

void main() {
    uint currentID = gl_GlobalInvocationID.x;
    if(currentID >= denseIDs.length()) {
        return;
    }

    uint currentPointID = denseIDs[currentID];
    vec3 particleForce = ComputeForce(currentPointID);
    force[forceIDs[currentPointID]].xyz = particleForce;
}