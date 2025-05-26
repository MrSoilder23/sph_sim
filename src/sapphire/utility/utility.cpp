#include "sapphire/utility/utility.hpp"

namespace sapphire {
    float CubicSplineKernel(float radius, float smoothingLength) {
        float q = radius / smoothingLength;
        if (q < 0) return 0.0f;
        if (q >= 2.0f) return 0.0f;

        const float norm = 1.0f / (PI * smoothingLength * smoothingLength * smoothingLength);
        if (q <= 1.0f) {
            return norm * (1.0f - 1.5f*q*q + 0.75f*q*q*q);
        } else {
            float t = 2.0f - q;
            return norm * 0.25f * t * t * t;
        }
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

    float CubicSplineLaplacian(float radius, float smoothingLength) {
        float q = radius / smoothingLength;
        if(q >= 2.0f) {
            return 0.0f;
        }
        if(q == 0.0f) {
            return 0.0f;
        }

        const float norm = 45.0f / (PI * smoothingLength * smoothingLength * smoothingLength * smoothingLength * smoothingLength);
        if(q <= 1.0f) {
            return norm * (1.0f - q);
        } else {
            return norm * (2.0f - q) * (2.0f - q) * (2.0f - q);
        }
    }
}
