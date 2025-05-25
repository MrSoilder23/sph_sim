#pragma once
// C++ standard libraries
#include <vector>

// Third_party libraries
#include <glm/glm.hpp>

namespace sapphire {
    constexpr float PI = 3.14159265358979323846; 

    float Poly6Kernel(float radiusSquared, float smoothingLength);

    float ComputeDensity(const glm::vec4& point, const std::vector<glm::vec4*>& neighbors, float smoothingLength, float mass);

    float CubicSplineDerivative(float radius, float smoothingLength);

    glm::vec3 CubicSplineGradient(const glm::vec3& deltaPoint, float smoothingLength);

    float QuinticKernel(float radius, float smoothingLength);
}