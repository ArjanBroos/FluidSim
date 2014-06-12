#pragma once

#include "particle.h"
#include <vector>

// Simulates fluids using particles
class FluidSimulator {
public:
	~FluidSimulator();

	// This class takes ownership of the particle pointers and will be the one to destroy them
	void AddParticle(Particle* particle);
	void AddParticles(const std::vector<Particle*>& particles);

	// Do an explicit Euler time integration step
	void ExplicitEulerStep(float dt);

	std::vector<Particle*>&	GetParticles();

private:
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

	void		CalculateDensities();
	void		CalculatePressures();

	void		ApplyAllForces();
	void		ApplyPressureForces();
	void		ApplyViscosityForces();
	void		ApplySurfaceTensionForces();

	std::vector<Particle*>	particles;	// These particles represent the fluid
};