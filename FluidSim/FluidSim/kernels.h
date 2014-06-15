#pragma once

#include <glm/glm.hpp>

// Takes |r|² as input, to avoid expensive sqrt() function
float		KernelPoly6(float lengthOfRSquared, float h);
glm::vec3	KernelPoly6Gradient(const glm::vec3& r, float h);
float		KernelPoly6Laplacian(const glm::vec3& r, float h);
float		KernelSpiky(const glm::vec3& r, float h);
glm::vec3	KernelSpikyGradient(const glm::vec3& r, float h);
float		KernelSpikyLaplacian(const glm::vec3& r, float h);
float		KernelViscosity(const glm::vec3& r, float h);
glm::vec3	KernelViscosityGradient(const glm::vec3& r, float h);
float		KernelViscosityLaplacian(const glm::vec3& r, float h);