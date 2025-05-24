#pragma once
// C++ standard libraries
#include <vector>

// Third_party libraries
#include <glm/glm.hpp>

namespace sapphire {
    constexpr float PI = 3.14159265358979323846; 

    float GaussianKernel2D(float radiusSquared, float smoothingLength) {
        float smoothingSquared = smoothingLength * smoothingLength;
        return std::exp(-0.5f * radiusSquared / smoothingSquared) / (2.0f * PI * smoothingSquared);
    }

    float ComputeDensity(const glm::vec4& point, const std::vector<glm::vec4*>& neighbors, float smoothingLength, float mass) {
        float density = 0.0f;

        for(const auto& neighbor : neighbors) {
            glm::vec3 diff = point - *neighbor;
            float radiusSquared = glm::dot(diff, diff);
            if(radiusSquared < smoothingLength * smoothingLength) {
                density += mass * GaussianKernel2D(radiusSquared, smoothingLength);
            }
        }

        return density;
    }

    float CubicSplineDerivative(float radius, float smoothingLength) {
        float normalizedDistance = radius / smoothingLength;
        if(normalizedDistance <= 0.0f || normalizedDistance >= 2.0f) {
            return 0.0f;
        }

        const float normalization = 1.0f / (PI * smoothingLength * smoothingLength * smoothingLength);
        
        if(normalizedDistance <= 1.0f) {
            return normalization * (-3.0f * normalizedDistance + 2.25f * normalizedDistance * normalizedDistance) / smoothingLength;
        } else {
            float factor = 2.0f - normalizedDistance;
            return -normalization * 0.75f * factor * factor / smoothingLength;
        }
    }

    glm::vec3 CubicSplineGradient(const glm::vec3& deltaPoint, float smoothingLength) {
        float radius = glm::length(deltaPoint);
        if(radius == 0.0f) {
            return glm::vec3(0.0f);
        }

        float dwdr = CubicSplineDerivative(radius, smoothingLength);
        return dwdr * (deltaPoint / radius);
    }

    float QuinticKernel(float radius, float smoothingLength) {
        float normalizedDistance = radius / smoothingLength;
        if(normalizedDistance <= 0.0f || normalizedDistance >= 2.0f) {
            return 0.0f;
        }

        const float normalization = 45.0f / (4.0f * PI * smoothingLength * smoothingLength * smoothingLength);

        float factor = 1.0f - normalizedDistance;
        return normalization * (1.0f + 5.0f * normalizedDistance + 8.0f * normalizedDistance * normalizedDistance) *
            factor * factor * factor * factor * factor;
    }
}