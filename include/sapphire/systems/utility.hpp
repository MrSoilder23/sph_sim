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
}