#include "fluidsimulator.h"

#include <algorithm>
#include <glm/gtx/norm.hpp>
#include "kernels.h"

const float h = 20.f;			// SPH radius
const float k = 1e8f;			// Pressure constant
const float mu = 3e4f;			// Viscosity constant
const float bounce = 0.3f;		// Collision response factor

FluidSimulator::FluidSimulator(const AABoundingBox& boundingBox) {
	this->boundingBox = boundingBox;
}

FluidSimulator::~FluidSimulator() {
	Clear();
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
	DetectAndRespondCollisions(dt);
}

// Removes all particles
void FluidSimulator::Clear() {
	// Free reserved memory
	for (auto pi = particles.begin(); pi != particles.end(); pi++)
		delete *pi;
	particles.clear();
}

std::vector<Particle*>&	FluidSimulator::GetParticles() {
	return particles;
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
			Particle* pj = particles[j];

			float rSquared = glm::length2(pi->position - pj->position);
			float kernel = KernelPoly6(rSquared, h);
			pi->density += particles[j]->mass * kernel;
			pj->density += particles[i]->mass * kernel;
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
	ApplyGravityForces();
	ApplyViscosityForces();
	ApplySurfaceTensionForces();
}

void FluidSimulator::ApplyPressureForces() {
	// For every particle
	for (unsigned i = 0; i < particles.size(); i++) {
		Particle* pi = particles[i];

		// Compute pressure force from every other particle
		for (unsigned j = i+1; j < particles.size(); j++) {
			Particle* pj = particles[j];

			if (abs(pj->density) < 1e-8f || abs(pi->density) < 1e-8f) continue; // Prevent division by 0

			const glm::vec3 r = pi->position - pj->position;
			if (glm::length(r) < 1e-8f) continue;

			pi->forceAccum -= pj->mass * ((pi->pressure + pj->pressure) / (2.f * pj->density)) * KernelSpikyGradient(r, h);
			pj->forceAccum -= pi->mass * ((pi->pressure + pj->pressure) / (2.f * pi->density)) * KernelSpikyGradient(-r, h);

			float t = 0.f;
		}
	}
}

void FluidSimulator::ApplyViscosityForces() {
	// For every particle
	for (unsigned i = 0; i < particles.size(); i++) {
		Particle* pi = particles[i];

		// Compute viscosity force from every other particle
		for (unsigned j = i+1; j < particles.size(); j++) {
			Particle* pj = particles[j];

			const glm::vec3 r = pi->position - pj->position;
			const glm::vec3 v = pj->velocity - pi->velocity;

			pi->forceAccum += mu * pj->mass * (v / pj->density) * KernelViscosityLaplacian(r, h);
			pj->forceAccum += mu * pi->mass * (-v / pi->density) * KernelViscosityLaplacian(-r, h);
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

void FluidSimulator::DetectAndRespondCollisions(float dt) {
	glm::vec3	cp;	// Point of collision
	float		d;	// Penetration depth
	glm::vec3	n;	// Normal at point of collision

	for (auto pi = particles.begin(); pi != particles.end(); pi++) {
		Particle* p = *pi;

		// If a collision was detected
		if (boundingBox.Outside(p->position, cp, d, n)) {
			glm::vec3 bounceV = p->velocity - (1.f + bounce) * glm::dot(p->velocity, n) * n;
			// Put particle back at contact point
			p->position = cp;
			// Reflect velocity with bounce factor in mind
			p->velocity = p->velocity - (1.f + bounce) * glm::dot(p->velocity, n) * n;
		}
	}
}