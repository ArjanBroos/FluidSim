#include "fluidsimulator.h"

#include <algorithm>
#include <glm/gtx/norm.hpp>
#include "kernels.h"
#include <windows.h>
#include <sstream>
#include <vector>
//#include <stdio.h>
#include <iostream>

const float h = 25.f;			// SPH radius
const float k = 3e8f;			// Pressure constant
const float mu = 6e4f;			// Viscosity constant
const float bounce = 0.2f;		// Collision response factor
const float sigma = 10.f;		// Surface tension coefficient

FluidSimulator::FluidSimulator(const AABoundingBox& boundingBox) {
	this->boundingBox = boundingBox;
	gravity = true;
	wind = false;
	surfaceTension = false;
	initOctree();
}

FluidSimulator::~FluidSimulator() {
	Clear();
}

void FluidSimulator::initOctree(){
	d1 = 3 + (int)floor(abs(boundingBox.right - boundingBox.left)/h);
	d2 = 3 + (int)floor(abs(boundingBox.top - boundingBox.bottom)/h);
	d3 = 3 + (int)floor(abs(boundingBox.front - boundingBox.back)/h);
	c1 = (min(boundingBox.right, boundingBox.left));
	c2 = (min(boundingBox.top, boundingBox.bottom));
	c3 = (min(boundingBox.front, boundingBox.back));

	this->octree.resize(d1*d2*d3);
	std::ostringstream os_;
	os_ << "initoctree : " << d1 << " " << d2 << " " << d3 << " " << this->octree.size();
	OutputDebugString(os_.str().c_str());
	//for (unsigned i=0;i<octree.size();i++)
		//this->octree[i].resize(0);
	OutputDebugString("initoctree : \n");

	/*for (auto pi = particles.begin(); pi != particles.end(); pi++){

		int pos = 1+(int)floor(((*pi)->position.x-c1)/h) + (1+(int)floor(((*pi)->position.y-c2)/h) + (1+(int)floor(((*pi)->position.z-c3)/h))*d2 )*d1;
		
		if (d1*d2*d3<=pos){
			OutputDebugString("Hello world!");
			std::cout << "hello ";
			continue;
		}

		(this->octree.at(pos)).push_back(*pi);
		(*pi)->hashOctree = pos;
		std::cout << "start ";
		std::cout << " ; " << pos;

		os_ << pos << " ; ";
		OutputDebugString(os_.str().c_str());
	}*/
}

void FluidSimulator::calculateOctree(){
	for (auto pi = particles.begin(); pi != particles.end(); pi++){

		int pos = 1+(int)floor(((*pi)->position.x-c1)/h) + (1+(int)floor(((*pi)->position.y-c2)/h) + (1+(int)floor(((*pi)->position.z-c3)/h))*d2 )*d1;
		if (pos == (*pi)->hashOctree) continue;
		/*std::ostringstream os_;
		os_ << "pos " << pos << " octrr: " << (this->octree).size();
		outputdebugstring(os_.str().c_str());*/
		
		/*if (d1*d2*d3<=pos){
			OutputDebugString("Hello world!");
			continue;
		}*/
		//char    buf[128];
		//sprintf(buf, "Var: %d", pos);
		//OutputDebugString(buf);
		/*std::cout << "Pos: " << pos << ";" << std::endl;
		std::cout << "size: " << this->octree.size() << std::endl;
		std::cout << "curPos: " << (*pi)->hashOctree << std::endl;
		if ((*pi)->hashOctree >= 0) std::cout << "octree(pos)Size" << (this->octree.at((*pi)->hashOctree)).size() << std::endl;
		std::cout << "4: " << std::endl;*/

		/*for (auto pip = this->octree.at((*pi)->hashOctree).begin(); pi != this->octree.at((*pi)->hashOctree).end(); pi++){
			std::cout << (*pip)->hashOctree << " - ";
		}
		std::cout << std::endl;*/

		//std::cout << ""<<(*pi)->hashOctree << " - " << std::endl;
		//std::cout << ""<<(((*pi)->hashOctree) >= 0) << " - " << std::endl;
		//std::cout << ""<<(*pi)->hashOctree << " - " << std::endl;

		if (((*pi)->hashOctree) >= 0){
			std::cout << (*pi)->hashOctree << std::endl;
			(this->octree.at((*pi)->hashOctree)).erase(std::remove(this->octree.at((*pi)->hashOctree).begin(), this->octree.at((*pi)->hashOctree).end(), (*pi)), this->octree.at((*pi)->hashOctree).end());
		}

		(this->octree.at(pos)).push_back(*pi);
		(*pi)->hashOctree = pos;
	}
	std::cout << std::endl;
}

void FluidSimulator::GetParticlesClose(Particle* pi, std::vector<Particle*>& particles){
	//int pos = 1+(int)floor(pi->position.x/h) + (1+(int)floor(pi->position.y/h) + (1+(int)floor(pi->position.z/h))*d2 )*d1;
	int pos = 1+(int)floor(((pi)->position.x-c1)/h) + (1+(int)floor(((pi)->position.y-c2)/h) + (1+(int)floor(((pi)->position.z-c3)/h))*d2 )*d1;
	int pos2;
	for (int i = -1; i <= 1; i++){
		for (int j = -1; j <= 1; j++){
			for (int k = -1; k <= 1; k++){
				pos2 = pos + i + (j + k*d2 )*d1;
				//int pos2 = 1+(int)floor(pi->position.x/h)+i + (1+(int)floor(pi->position.y/h)+j + (1+(int)floor(pi->position.z/h)+k)*d2 )*d1;
				/*std::ostringstream os_;
				os_ << "pos2 " << pos2 << " ijk: " << i << " " << j << " " << k << " posHash: " << (int)floor(pi->position.x/h) << " " << (int)floor(pi->position.y/h) << " " << (int)floor(pi->position.z/h);
				OutputDebugString(os_.str().c_str());*/
				for (auto p2 = octree.at(pos2).begin(); p2 != octree.at(pos2).end(); p2++)
					particles.push_back(*p2);
				
				particles.reserve( particles.size() + octree.at(pos2).size() ); // preallocate memory
				particles.insert( particles.end(), octree.at(pos2).begin(), octree.at(pos2).end() );
			}
		}
	}
	//particles = this->particles;
}

// This class takes ownership of the particle pointers and will be the one to destroy them
void FluidSimulator::AddParticle(Particle* particle) {
	particles.push_back(particle);
}

void FluidSimulator::AddParticles(const std::vector<Particle*>& particles) {
	for (auto pi = particles.begin(); pi != particles.end(); pi++)
		this->particles.push_back(*pi);
}

void FluidSimulator::ToggleGravity() {
	gravity = !gravity;
}

void FluidSimulator::ToggleWind() {
	wind = !wind;
}

void FluidSimulator::ToggleUseOctree() {
	useOctree = !useOctree;
}

void FluidSimulator::ToggleSurfaceTension(){
	surfaceTension = !surfaceTension;
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

// Removes all particles from octree
void FluidSimulator::clearOctree() {
	// Free reserved memory
	for (auto pi = octree.begin(); pi != octree.end(); pi++)
		(*pi).clear();
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

	if (useOctree){
		calculateOctree();
	}
	ApplyPressureForces();
	if (gravity) ApplyGravityForces();
	if (wind) ApplyWindForces();
	ApplyViscosityForces();
	if (surfaceTension) ApplySurfaceTensionForces();
}

void FluidSimulator::ApplyPressureForces() {
	if (!useOctree){
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
			}
		}
	}else{
		std::vector<Particle*> closeParticles;
		// For every particle
		for (unsigned i = 0; i < particles.size(); i++) {
			Particle* pi = particles[i];

			GetParticlesClose(pi, closeParticles);
			// Compute pressure force from every other particle
			for (unsigned j = 0; j < closeParticles.size(); j++) {
				Particle* pj = closeParticles[j];

				if (abs(pj->density) < 1e-8f || abs(pi->density) < 1e-8f) continue; // Prevent division by 0

				const glm::vec3 r = pi->position - pj->position;
				if (glm::length(r) < 1e-8f) continue;

				pi->forceAccum -= pj->mass * ((pi->pressure + pj->pressure) / (2.f * pj->density)) * KernelSpikyGradient(r, h);
			}
		}
	}
}

void FluidSimulator::ApplyViscosityForces() {
	if (!useOctree){
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
	}else{
		std::vector<Particle*> closeParticles;
		// For every particle
		for (unsigned i = 0; i < particles.size(); i++) {
			Particle* pi = particles[i];

			GetParticlesClose(pi, closeParticles);
			// Compute viscosity force from every particle close by
			for (unsigned j = 0; j < closeParticles.size(); j++) {
				Particle* pj = closeParticles[j];

				const glm::vec3 r = pi->position - pj->position;
				const glm::vec3 v = pj->velocity - pi->velocity;

				pi->forceAccum += mu * pj->mass * (v / pj->density) * KernelViscosityLaplacian(r, h);
			}
		}
	}
}

void FluidSimulator::ApplySurfaceTensionForces() {
	float lenThreshold = 1e-8f;
	bool notOctree = true;
	if (!useOctree || notOctree){
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
	}else{
		std::vector<Particle*> closeParticles;
		for (unsigned i = 0; i < particles.size(); i++) {
			Particle* pi = particles[i];

			glm::vec3 gradCs = glm::vec3(0,0,0);
			float laplaceCs = 0;

			GetParticlesClose(pi, closeParticles);
			// Compute surfaceTension force from every other particle
			for (unsigned j = 0; j < closeParticles.size(); j++) {
				if (j != i){
					Particle* pj = closeParticles[j];
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
}

void FluidSimulator::ApplyGravityForces() {
	const float g = 9.81f; // Gravitational constant for Earth
	const glm::vec3 gv(0.f, -g, 0.f);
	for (auto pi = particles.begin(); pi != particles.end(); pi++) {
		Particle* p = *pi;
		p->forceAccum += p->restDensity * gv;
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
	}
}

float FluidSimulator::csGradient(float cs) {
	return 0;
}

bool FluidSimulator::isWind(){
	return wind;
}
bool FluidSimulator::isGravity(){
	return gravity;
}
bool FluidSimulator::isSurfaceTension(){
	return surfaceTension;
}
bool FluidSimulator::isUseOctree(){
	return useOctree;
}