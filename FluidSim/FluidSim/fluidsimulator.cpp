#include "fluidsimulator.h"

#include <algorithm>
#include <glm/gtx/norm.hpp>
#include "kernels.h"

const float h = 25.f;			// SPH radius
const float k = 3e8f;			// Pressure constant
const float mu = 6e4f;			// Viscosity constant
const float bounce = 0.2f;		// Collision response factor
const float sigma = 10.f;		// Surface tension coefficient

FluidSimulator::FluidSimulator(const AABoundingBox& boundingBox) {
	this->boundingBox = boundingBox;
	fluidgravity = true;
	bodygravity = false;
	wind = false;
	surfaceTension = false;
}

FluidSimulator::~FluidSimulator() {
	Clear();
}

void FluidSimulator::initOctree(){
	d1 = 2 + (int)floor(abs(boundingBox.right - boundingBox.left)/h);
	d1 = 2 + (int)floor(abs(boundingBox.top - boundingBox.bottom)/h);
	d1 = 2 + (int)floor(abs(boundingBox.front - boundingBox.back)/h);
	this->octree.resize((d1+2)*(d2+2)*(d3+2));
}

void FluidSimulator::calculateOctree(){
	for (auto pi = particles.begin(); pi != particles.end(); pi++){
		int pos = 1+(int)floor((*pi)->position.x/h) + (1+(int)floor((*pi)->position.y/h) + (1+(int)floor((*pi)->position.z/h))*d2 )*d1;
		this->octree[pos].push_back(*pi);
	}
}

std::vector<Particle*> FluidSimulator::GetParticles(Particle* pi, std::vector<Particle*> particles){
	int pos = 1+(int)floor(pi->position.x/h) + (1+(int)floor(pi->position.y/h) + (1+(int)floor(pi->position.z/h))*d2 )*d1;
	int pos2;
	for (int i = -1; i <= 1; i++){
		for (int j = -1; j <= 1; j++){
			for (int k = -1; k <= 1; k++){
				pos2 = pos + i + (j + k*d2 )*d1;
				for (auto p2 = octree[pos2].begin(); p2 != octree[pos2].end(); p2++)
					particles.push_back(*p2);
			}
		}
	}
}

// This class takes ownership of the particle pointers and will be the one to destroy them
void FluidSimulator::AddParticle(Particle* particle) {
	particles.push_back(particle);
}

void FluidSimulator::AddParticles(const std::vector<Particle*>& particles) {
	for (auto pi = particles.begin(); pi != particles.end(); pi++)
		this->particles.push_back(*pi);
}

// This class takes ownership of the body pointers and will be the one to destroy them
void FluidSimulator::AddBody(body* body) {
	bodies.push_back(body);
	movingBody = body;
}

void FluidSimulator::AddBodies(const std::vector<body*>& bodies) {
	for (auto pi = bodies.begin(); pi != bodies.end(); pi++)
		this->bodies.push_back(*pi);
}

void FluidSimulator::ToggleFluidGravity() {
	fluidgravity = !fluidgravity;
}

void FluidSimulator::ToggleBodyGravity() {
	bodygravity = !bodygravity;
}

void FluidSimulator::ToggleWind() {
	wind = !wind;
}

void FluidSimulator::ToggleSurfaceTension(){
	surfaceTension = !surfaceTension;
}

// Do an explicit Euler time integration step
void FluidSimulator::ExplicitEulerStep(float dt) {
	// Clear force accumulators
	for (auto pi = particles.begin(); pi != particles.end(); pi++)
		(*pi)->forceAccum = glm::vec3(0.f, 0.f, 0.f);
	for (auto bi = bodies.begin(); bi != bodies.end(); bi++)
		(*bi)->forceAccum = glm::vec3(0.f, 0.f, 0.f);

	// Apply forces
	ApplyAllForces();

	// Update positions and velocity
	for (auto pi = particles.begin(); pi != particles.end(); pi++) {
		Particle* p = *pi;
		p->position += p->velocity * dt;
		p->velocity += (p->forceAccum / p->mass) * dt;
	}
	for (auto bi = bodies.begin(); bi != bodies.end(); bi++) {
		body* b = *bi;
		b->position += b->velocity * dt;
		b->velocity += (b->forceAccum / b->mass) * dt;
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
	// Free reserved memory
	for (auto pi = bodies.begin(); pi != bodies.end(); pi++)
		delete *pi;
	bodies.clear();
}

std::vector<Particle*>&	FluidSimulator::GetParticles() {
	return particles;
}

std::vector<body*>&	FluidSimulator::GetBodies() {
	return bodies;
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
	if (wind) ApplyWindForces();
	ApplyViscosityForces();
	if (surfaceTension) ApplySurfaceTensionForces();
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
	float lenThreshold = 1e-8f;
	// For every particle
	for (unsigned i = 0; i < particles.size(); i++) {
		Particle* pi = particles[i];

		glm::vec3 gradCs = glm::vec3(0,0,0);
		float laplaceCs = 0;

		// Compute surfaceTension force from every other particle
		for (unsigned j = 0; j < particles.size(); j++) {
			if (j != i){
				Particle* pj = particles[j];
				const glm::vec3 r = pi->position - pj->position;
				float laplace = 0;

				float rSquared = glm::length2(pi->position - pj->position);
				glm::vec3 grad = KernelPoly6GradientLaplacian(r,h,laplace);

				gradCs += pj->mass / pj->density * grad;
				laplaceCs += pj->mass / pj->density * laplace;

			}
		}
		float nlen = glm::length(gradCs);

		if (nlen < lenThreshold) continue;

		pi->forceAccum += -sigma * laplaceCs * gradCs / nlen;
	}
}

void FluidSimulator::ApplyGravityForces() {
	const float g = 9.81f; // Gravitational constant for Earth
	const glm::vec3 gv(0.f, -g, 0.f);
	if (fluidgravity){
		for (auto pi = particles.begin(); pi != particles.end(); pi++) {
			Particle* p = *pi;
			p->forceAccum += p->restDensity * gv;
		}
	}
	if (bodygravity){
		for (auto bi = bodies.begin(); bi != bodies.end(); bi++) {
			body* b = *bi;
			b->forceAccum += b->mass * gv;
		}
	}
}

// Apply a force to all particles in direction of negative x-axis
void FluidSimulator::ApplyWindForces() {
	const float wf = 6.f;
	const glm::vec3 wfv(-wf, 0.f, 0.f);
	for (auto pi = particles.begin(); pi != particles.end(); pi++) {
		Particle* p = *pi;
		p->forceAccum += p->restDensity * wfv;
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

		/*if (box.Inside(p->position, cp, d, n)) {
			glm::vec3 bounceV = p->velocity - (1.f + bounce) * glm::dot(p->velocity, n) * n;
			// Put particle back at contact point
			p->position = cp;
			// Reflect velocity with bounce factor in mind
			p->velocity = p->velocity - (1.f + bounce) * glm::dot(p->velocity, n) * n + box.velocity;
		}*/
	}

	static const float sElasticity = bounce;
	static const float sImpactCoefficient = 1.0f + sElasticity;
	for (auto bi = bodies.begin(); bi != bodies.end(); bi++) {
		body* rigidBody = *bi;
		const glm::vec3 & physObjVelocity = rigidBody->GetVelocity();
		for (auto pi = particles.begin(); pi != particles.end(); pi++) {
			Particle* particle = *pi;
			if (rigidBody->collision(particle->position, cp, d, n)){
				const glm::vec3  vVelDueToRotAtConPt = rigidBody->GetAngularVelocity(cp);
				const glm::vec3  vVelBodyAtConPt = physObjVelocity + vVelDueToRotAtConPt;
				const glm::vec3  velRelative = particle->velocity - vVelBodyAtConPt;
				const glm::vec3  speedNormal = velRelative * n; // Contact normal depends on geometry.
				const glm::vec3  impulse = -speedNormal * n; // Minus: speedNormal is negative.
				particle->velocity = particle->velocity + impulse * sImpactCoefficient;
				particle->position = rigidBody->absoluteContactPoint(cp);
			}
		}
	}

}

float FluidSimulator::csGradient(float cs) {
	return 0;
}

bool FluidSimulator::isWind(){
	return wind;
}
bool FluidSimulator::isGravity(){
	return fluidgravity;
}
bool FluidSimulator::isSurfaceTension(){
	return surfaceTension;
}