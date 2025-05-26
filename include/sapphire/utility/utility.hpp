#pragma once
// C++ standard libraries
#include <vector>

// Third_party libraries
#include <glm/glm.hpp>

namespace sapphire {
    constexpr float PI = 3.14159265358979323846;

    float CubicSplineKernel(const float& radius, const float& smoothingLength);
    float CubicSplineDerivative(const float& radius, const float& smoothingLength);
    glm::vec3 CubicSplineGradient(const glm::vec3& deltaPoint, const float& smoothingLength);

    float CubicSplineLaplacian(const float& radius, const float& smoothingLength);
}