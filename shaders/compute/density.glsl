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

// Uniforms
uniform float uStiffness;
uniform float uRestDensity;
uniform float uSmoothingLength;


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

float ComputeDensity(uint currentSphereID) {
    float density = 0.0f;

    vec3 point = positionAndRadius[sphereIDs[currentSphereID]].xyz;
    float radiusSquared = uSmoothingLength*uSmoothingLength;

    for(uint i = 0; i < sphereIDs.length(); i++) {
        uint diffSphereID = denseIDs[i];

        vec3 diff = point - positionAndRadius[sphereIDs[diffSphereID]].xyz;
        float radius = length(diff);

        if(radius <= radiusSquared) {
            density += mass[massIDs[diffSphereID]] * CubicSplineKernel(radius);
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