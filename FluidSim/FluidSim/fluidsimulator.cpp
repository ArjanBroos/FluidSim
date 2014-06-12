#include "fluidsimulator.h"

#include <algorithm>
#define PI 3.141592654f
#include <glm/gtx/norm.hpp>

const float h = 50.f;			// SPH radius
const float k = 1e6f;			// Pressure constant
const float mu = 10.f;			// Viscosity constant
const float bounce = 0.01f;		// Collision response factor

FluidSimulator::FluidSimulator(const AABoundingBox& boundingBox) {
	this->boundingBox = boundingBox;
}

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

	// Fix collisions
	DetectAndRespondCollisions();
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
	// Clear density
	for (auto pi = particles.begin(); pi != particles.end(); pi++)
		(*pi)->density = (*pi)->restDensity;

	// For every particle
	for (unsigned i = 0; i < particles.size(); i++)  {
		Particle* pi = particles[i];

		// Calculate density from every other particle
		for (unsigned j = i+1; j < particles.size(); j++) {
			if (i == j) continue;
			Particle* pj = particles[j];

			float rSquared = glm::length2(pi->position - pj->position);
			float density = particles[j]->mass * KernelPoly6(rSquared, h);
			pi->density += density;
			pj->density += density;
		}
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
	ApplyGravityForces();
}

void FluidSimulator::ApplyPressureForces() {
	// For every particle
	for (unsigned i = 0; i < particles.size(); i++) {
		Particle* pi = particles[i];

		// Compute pressure force from every other particle
		for (unsigned j = i+1; j < particles.size(); j++) {
			if (i == j) continue;
			Particle* pj = particles[j];

			if (abs(pj->density) < 1e-8f) continue; // Prevent division by 0

			const glm::vec3 r = pi->position - pj->position;
			if (glm::length2(r) > h*h) continue;

			const glm::vec3 pressureForce = pj->mass *
				((pi->pressure + pj->pressure) / (2.f * pj->density)) *
				KernelSpikyGradient(r, h);

			pi->forceAccum -= pressureForce;
			pj->forceAccum += pressureForce;
		}
	}
}

void FluidSimulator::ApplyViscosityForces() {
	// For every particle
	for (unsigned i = 0; i < particles.size(); i++) {
		Particle* pi = particles[i];

		// Compute viscosity force from every other particle
		for (unsigned j = 0; j < particles.size(); j++) {
			if (i == j) continue;
			Particle* pj = particles[j];

			const glm::vec3 r = pi->position - pj->position;
			const glm::vec3 v = pj->velocity - pi->velocity;

			const glm::vec3 viscosityForce = pj->mass *
				(v / pj->density) *
				KernelViscosityLaplacian(r, h);

			pi->forceAccum += mu * viscosityForce;
			pj->forceAccum -= mu * viscosityForce;
		}
	}
}

void FluidSimulator::ApplySurfaceTensionForces() {
}

void FluidSimulator::ApplyGravityForces() {
	const float g = 9.81f; // Gravitational constant for Earth
	const glm::vec3 gv(0.f, -g, 0.f);
	for (auto pi = particles.begin(); pi != particles.end(); pi++) {
		Particle* p = *pi;
		p->forceAccum += p->restDensity * gv;
	}
}

void FluidSimulator::DetectAndRespondCollisions() {
	glm::vec3	cp;	// Point of collision
	glm::vec3	n;	// Normal at point of collision

	for (auto pi = particles.begin(); pi != particles.end(); pi++) {
		Particle* p = *pi;

		// If a collision was detected
		if (boundingBox.Outside(p->position, cp, n)) {
			glm::vec3 bounceV = p->velocity - (1.f + bounce) * glm::dot(p->velocity, n) * n;
			// Put particle back at contact point
			p->position = cp;
			// Reflect velocity with bounce factor in mind
			p->velocity = p->velocity - (1.f + bounce) * glm::dot(p->velocity, n) * n;
		}
	}
}