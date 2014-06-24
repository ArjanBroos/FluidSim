#pragma once

#include "particle.h"
#include <vector>
#include "boundingbox.h"

// Simulates fluids using particles
class FluidSimulator {
public:
	FluidSimulator(const AABoundingBox& boundingBox);
	~FluidSimulator();

	// This class takes ownership of the particle pointers and will be the one to destroy them
	void AddParticle(Particle* particle);
	void AddParticles(const std::vector<Particle*>& particles);

	void ToggleGravity();
	void ToggleWind();
	void ToggleSurfaceTension();

	// Do an explicit Euler time integration step
	void ExplicitEulerStep(float dt);

	// Removes all particles
	void Clear();

	std::vector<Particle*>&	GetParticles();

	bool isWind();
	bool isGravity();
	bool isSurfaceTension();

	//AABoundingBox			box;

private:
	void		CalculateDensities();
	void		CalculatePressures();

	void		ApplyAllForces();
	void		ApplyPressureForces();
	void		ApplyViscosityForces();
	void		ApplySurfaceTensionForces();
	void		ApplyGravityForces();
	void		ApplyWindForces();

	void		DetectAndRespondCollisions(float dt);
	float		csGradient(float cs);

	void FluidSimulator::initOctree();
	void FluidSimulator::calculateOctree();
	std::vector<Particle*> FluidSimulator::GetParticles(Particle* pi, std::vector<Particle*> particles);

	AABoundingBox			boundingBox;	// The bounding box in which the particles should reside
	std::vector<Particle*>	particles;		// These particles represent the fluid
	std::vector<std::vector<Particle*>>	octree; // octree for detecting particles close to one another
	bool					gravity;		// True if gravity force is to be applied
	bool					wind;			// True if wind force is to be applied
	bool					surfaceTension;	// True if surface tension force is to be applied
	
	int						d1,d2,d3;		// dimensions of octree (including extra (empty) space on each side)
};