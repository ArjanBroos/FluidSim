#include "kernels.h"

#define PI 3.141592654f

// Takes |r|², to avoid using expensive sqrt() function
float KernelPoly6(float lengthOfRSquared, float h) {
	const float hs = h*h;
	if (lengthOfRSquared < 0.f || lengthOfRSquared > hs) return 0.f;
	return (315.f / (64.f * PI * pow(h, 9))) * pow(hs - lengthOfRSquared, 3);
}

glm::vec3 KernelPoly6Gradient(const glm::vec3& r, float h) {
	const float lr = glm::length(r);
	const float lrs = lr*lr;
	const float hs = h*h;
	if (lr < 0.f || lr > h) return glm::vec3(0.f, 0.f, 0.f);
	return (-945.f / (32.f * PI * pow(h, 9))) * r * pow(hs - lrs, 2);
}

float KernelPoly6Laplacian(const glm::vec3& r, float h) {
	const float lr = glm::length(r);
	const float lrs = lr*lr;
	const float hs = h*h;
	if (lr < 0.f || lr > h) return 0.f;
	return (-945.f / (32 * PI * pow(h, 9))) * (hs - lrs) * (3.f * hs - 7.f * lrs);
}

float KernelSpiky(const glm::vec3& r, float h) {
	const float lr = glm::length(r);
	if (lr < 0.f || lr > h) return 0.f;
	return (15.f / (PI * pow(h, 6))) * pow(h - lr, 3);
}

glm::vec3 KernelSpikyGradient(const glm::vec3& r, float h) {
	const float lr = glm::length(r);
	if (lr < 0.f || lr > h) return glm::vec3(0.f, 0.f, 0.f);
	return (-45.f / (PI * pow(h, 6))) * (r / lr) * pow(h - lr, 2);
}

float KernelSpikyLaplacian(const glm::vec3& r, float h) {
	const float lr = glm::length(r);
	if (lr < 0.f || lr > h) return 0.f;
	return (-90.f / (PI * pow(h, 6))) * (1.f / lr) * (h - lr) * (h - 2.f * lr);
}

float KernelViscosity(const glm::vec3& r, float h) {
	const float lr = glm::length(r);
	if (lr < 0.f || lr > h) return 0.f;
	const float term = (-pow(lr, 3) / (2.f * pow(h, 3))) +
		(lr*lr / h*h) + (h / (2.f * lr)) - 1.f;
	return (15.f / (2.f * PI * pow(h, 3))) * term;
}

glm::vec3 KernelViscosityGradient(const glm::vec3& r, float h) {
	const float lr = glm::length(r);
	if (lr < 0.f || lr > h) return glm::vec3(0.f, 0.f, 0.f);
	const float term = (-3.f * lr) / (2.f * pow(h, 3)) +
		2.f / (h*h) - h / (2.f * pow(lr, 3));
	return (15.f / (2.f * PI * pow(h, 3))) * r * term;
}

float KernelViscosityLaplacian(const glm::vec3& r, float h) {
	const float lr = glm::length(r);
	if (lr < 0.f || lr > h) return 0.f;
	return (45.f / (PI * pow(h, 6))) * (h - lr);
}