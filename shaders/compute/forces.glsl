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

// Uniforms
uniform float uSmoothingLength;
uniform float uG;


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

// Main Calculations
vec3 ComputeForce(uint currentID) {
    float softening = uSmoothingLength * 0.01f;
    float softeningSquared = softening*softening;

    vec3 pressureForce  = vec3(0.0f);
    vec3 viscosityForce = vec3(0.0f);
    vec3 gravityForce   = vec3(0.0f);

    // Current point data
    vec3 currentPoint          = positionAndRadius[sphereIDs[currentID]].xyz;
    vec3 currentPointVelocity  = velocity[velocityIDs[currentID]].xyz;
    float currentPointPressure = pressure[pressureIDs[currentID]];
    float currentPointDensity  = densities[densityIDs[currentID]];

    for(int i = 0; i < denseIDs.length(); i++) {
        uint diffSphereID = denseIDs[i];

        vec3 deltaPoint = currentPoint - positionAndRadius[sphereIDs[diffSphereID]].xyz;
        float radiusSquared = dot(deltaPoint, deltaPoint);
        float radius = sqrt(radiusSquared);

        if(radius > 0.0f && radius < uSmoothingLength) {
            float neighborDensity  = densities[densityIDs[diffSphereID]];
            float neighborPressure = pressure[pressureIDs[diffSphereID]];
            float neighborMass     = mass[massIDs[diffSphereID]];
            vec3 neighborVelocity  = velocity[velocityIDs[diffSphereID]].xyz;

            // Pressure
            float pressureTerm = (currentPointPressure / (currentPointDensity*currentPointDensity)) +
                (neighborPressure / (neighborDensity*neighborDensity));
            pressureForce += -pressureTerm * CubicSplineGradient(deltaPoint, radius);

            // Viscosity
            viscosityForce += (neighborDensity * (neighborVelocity - currentPointVelocity)) * CubicSplineLaplacian(radius);

            // Gravity
            float distSoft = radiusSquared + softeningSquared;
            float denominator = sqrt(distSoft*distSoft*distSoft);
            gravityForce += uG * neighborMass * deltaPoint / denominator;
        }
    }

    return pressureForce + viscosityForce + gravityForce;
}

void main() {
    uint currentID = gl_GlobalInvocationID.x;
    if(currentID >= denseIDs.length()) {
        return;
    }

    if(currentID >= 7500 && force.length() == 7500) {
        positionAndRadius[sphereIDs[currentID]] = vec4(0.0f, 10.0f, -40.0f, 1.0f);
    }


    uint currentPointID = denseIDs[currentID];
    vec3 particleForce = ComputeForce(currentPointID);
    force[forceIDs[currentPointID]].xyz = particleForce;
}