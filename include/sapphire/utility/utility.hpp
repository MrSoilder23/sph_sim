#pragma once
// C++ standard libraries
#include <vector>

// Third_party libraries
#include <glm/glm.hpp>

namespace sapphire {
    constexpr float PI = 3.14159265358979323846;

    float CubicSplineKernel(float radius, float smoothingLength);
    float CubicSplineDerivative(float radius, float smoothingLength);
    glm::vec3 CubicSplineGradient(const glm::vec3& deltaPoint, float smoothingLength);

    float CubicSplineLaplacian(float radius, float smoothingLength);
}