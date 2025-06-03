#pragma once
// C++ standard libraries
#include <vector>

// Third_party libraries
#include <glm/glm.hpp>

namespace sapphire {
    constexpr float PI = 3.14159265358979323846;

    float CubicSplineKernel(const float& radius, const float& smoothingLength);
    float CubicSplineDerivative(const float& radius, const float& smoothingLength);
    
    glm::vec3 CubicSplineGradient(const glm::vec3& deltaPoint, const float& radius, const float& smoothingLength);
    float CubicSplineLaplacian(const float& radius, const float& smoothingLength);

    inline float Dot(const glm::vec3& a, const glm::vec3& b) {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }

    inline float Length(const glm::vec3& a, const glm::vec3& b) {
        return std::sqrt(Dot(a,b));
    }
}