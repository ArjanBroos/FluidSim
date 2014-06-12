#include "fluidsimulator.h"

#include <algorithm>
#define PI 3.141592654f
#include <glm/gtx/norm.hpp>

const float h = 50.f;
const float k = 10000.f;
const float mu = 1.f;

FluidSimulator::~FluidSimulator() {
	// Free reserved memory
	for (auto pi = particles.begin(); pi != particles.end(); pi++)
		delete *pi;
	particles.clear();
}

// This class takes ownership of the particle pointers and will be the one to destroy them
void FluidSimulator::AddParticle(Particle* particle) {
	particles.push_back(particle);
}

void FluidSimulator::AddParticles(const std::vector<Particle*>& particles) {
	for (auto pi = particles.begin(); pi != particles.end(); pi++)
		this->particles.push_back(*pi);
}

// Do an explicit Euler time integration step
void FluidSimulator::ExplicitEulerStep(float dt) {
	// Clear force accumulators
	for (auto pi = particles.begin(); pi != particles.end(); pi++)
		(*pi)->forceAccum = glm::vec3(0.f, 0.f, 0.f);

	// Apply forces
	ApplyAllForces();

	// Update positions and velocity
	for (auto pi = particles.begin(); pi != particles.end(); pi++) {
		Particle* p = *pi;
		p->position += p->velocity * dt;
		p->velocity += (p->forceAccum / p->mass) * dt;
	}
}

std::vector<Particle*>&	FluidSimulator::GetParticles() {
	return particles;
}

// Takes |r|², to avoid using expensive sqrt() function
float FluidSimulator::KernelPoly6(float lengthOfRSquared, float h) {
	const float hs = h*h;
	if (lengthOfRSquared < 0.f || lengthOfRSquared > hs) return 0.f;
	return (315.f / (64.f * PI * pow(h, 9))) * pow(hs - lengthOfRSquared, 3);
}

glm::vec3 FluidSimulator::KernelPoly6Gradient(const glm::vec3& r, float h) {
	const float lr = glm::length(r);
	const float lrs = lr*lr;
	const float hs = h*h;
	if (lr < 0.f || lr > h) return glm::vec3(0.f, 0.f, 0.f);
	return (-945.f / (32.f * PI * pow(h, 9))) * r * pow(hs - lrs, 2);
}

float FluidSimulator::KernelPoly6Laplacian(const glm::vec3& r, float h) {
	const float lr = glm::length(r);
	const float lrs = lr*lr;
	const float hs = h*h;
	if (lr < 0.f || lr > h) return 0.f;
	return (-945.f / (32 * PI * pow(h, 9))) * (hs - lrs) * (3.f * hs - 7.f * lrs);
}

float FluidSimulator::KernelSpiky(const glm::vec3& r, float h) {
	const float lr = glm::length(r);
	if (lr < 0.f || lr > h) return 0.f;
	return (15.f / (PI * pow(h, 6))) * pow(h - lr, 3);
}

glm::vec3 FluidSimulator::KernelSpikyGradient(const glm::vec3& r, float h) {
	const float lr = glm::length(r);
	if (lr < 0.f || lr > h) return glm::vec3(0.f, 0.f, 0.f);
	return (-45.f / (PI * pow(h, 6))) * (r / lr) * pow(h - lr, 2);
}

float FluidSimulator::KernelSpikyLaplacian(const glm::vec3& r, float h) {
	const float lr = glm::length(r);
	if (lr < 0.f || lr > h) return 0.f;
	return (-90.f / (PI * pow(h, 6))) * (1.f / lr) * (h - lr) * (h - 2.f * lr);
}

float FluidSimulator::KernelViscosity(const glm::vec3& r, float h) {
	const float lr = glm::length(r);
	if (lr < 0.f || lr > h) return 0.f;
	const float term = (-pow(lr, 3) / (2.f * pow(h, 3))) +
		(lr*lr / h*h) + (h / (2.f * lr)) - 1.f;
	return (15.f / (2.f * PI * pow(h, 3))) * term;
}

glm::vec3 FluidSimulator::KernelViscosityGradient(const glm::vec3& r, float h) {
	const float lr = glm::length(r);
	if (lr < 0.f || lr > h) return glm::vec3(0.f, 0.f, 0.f);
	const float term = (-3.f * lr) / (2.f * pow(h, 3)) +
		2.f / (h*h) - h / (2.f * pow(lr, 3));
	return (15.f / (2.f * PI * pow(h, 3))) * r * term;
}

float FluidSimulator::KernelViscosityLaplacian(const glm::vec3& r, float h) {
	const float lr = glm::length(r);
	if (lr < 0.f || lr > h) return 0.f;
	return (45.f / (PI * pow(h, 6))) * (h - lr);
}

void FluidSimulator::CalculateDensities() {
	// For every particle
	for (unsigned i = 0; i < particles.size(); i++)  {
		Particle* pi = particles[i];
		float density = 0.f;

		// Calculate density from every other particle
		for (unsigned j = 0; j < particles.size(); j++) {
			if (i == j) continue;
			Particle* pj = particles[j];

			float rSquared = glm::length2(pi->position - pj->position);
			density += particles[j]->mass * KernelPoly6(rSquared, h);
		}

		particles[i]->density = density;
	}
}

void FluidSimulator::CalculatePressures() {
	for (auto pi = particles.begin(); pi != particles.end(); pi++) {
		Particle* p = *pi;
		p->pressure = k * (p->density - p->restDensity);
	}
}

void FluidSimulator::ApplyAllForces() {
	CalculateDensities();
	CalculatePressures();

	ApplyPressureForces();
	ApplyViscosityForces();
	ApplySurfaceTensionForces();
}

void FluidSimulator::ApplyPressureForces() {
	// For every particle
	for (unsigned i = 0; i < particles.size(); i++) {
		Particle* pi = particles[i];
		glm::vec3 pressureForce(0.f, 0.f, 0.f);

		// Compute pressure force from every other particle
		for (unsigned j = 0; j < particles.size(); j++) {
			if (i == j) continue;
			Particle* pj = particles[j];
			if (abs(pj->density) < 1e-10f) continue; // Prevent division by 0
			const glm::vec3 r = pi->position - pj->position;
			pressureForce += pj->mass *
				((pi->pressure + pj->pressure) / (2.f * pj->density)) *
				KernelSpikyGradient(r, h);
		}

		pi->forceAccum -= pressureForce;
	}
}

void FluidSimulator::ApplyViscosityForces() {
	// For every particle
	for (unsigned i = 0; i < particles.size(); i++) {
		Particle* pi = particles[i];
		glm::vec3 viscosityForce(0.f, 0.f, 0.f);

		// Compute viscosity force from evry other particle
		for (unsigned j = 0; j < particles.size(); j++) {
			if (i == j) continue;
			Particle* pj = particles[j];
			const glm::vec3 r = pi->position - pj->position;
			const glm::vec3 v = pj->velocity - pi->velocity;

			viscosityForce += pj->mass *
				(v / pj->density) *
				KernelViscosityLaplacian(r, h);
		}

		pi->forceAccum += mu * viscosityForce;
	}
}

void FluidSimulator::ApplySurfaceTensionForces() {
}