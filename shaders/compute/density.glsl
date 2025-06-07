#version 450 core

// Dense components array
layout(std430, binding = 0) buffer sphereComponent      { vec4  positionAndRadius[]; };
layout(std430, binding = 1) buffer pressureComponent    { float pressure[]; };
layout(std430, binding = 2) buffer densityComponent     { float density[];  };
layout(std430, binding = 3) buffer massComponent        { float mass[];     };

// Component locations in dense array
layout(std430, binding = 4) buffer sphereComponentLoc   { uint IDs[]; };
layout(std430, binding = 5) buffer massComponentLoc     { uint IDs[]; };
layout(std430, binding = 6) buffer pressureComponentLoc { uint IDs[]; };
layout(std430, binding = 7) buffer densityComponentLoc  { uint IDs[]; };

layout(std430, binding = 8) buffer neighborIDs          { uint IDs[]; };

uniform float uStiffness;
uniform float uRestDensity;
uniform float uSmoothingLength;

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

    vec3 point = positionAndRadius[sphereComponentLoc.IDs[currentSphereID]].xyz;

    for(int i = 0; i < neighborIDs.IDs.length(); i++) {
        uint neighborID = neighborIDs.IDs[i];
        vec3 diff = point - positionAndRadius[sphereComponentLoc.IDs[neighborID]].xyz;
        float radius = length(diff);

        density += mass[massComponentLoc.IDs[neighborID]] * CubicSplineKernel(radius);
    }

    return max(density, 1e-5f);
}

float ComputePressure(float density) {
    return uStiffness * (density - uRestDensity);
}

void main() {
    uint currentID = gl_GlobalInvocationID.x;
    if(currentID >= sphereComponentLoc.IDs.length()) {
        return;
    }

    float density = ComputeDensity(currentID);

    density[densityComponentLoc.IDs[currentID]] = density;
    pressure[pressureComponentLoc.IDs[currentID]] = ComputePressure(density);
}