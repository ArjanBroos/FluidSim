#pragma once

#include "particle.h"
#include "Body.h"
#include "Sphere.h"
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

	// This class takes ownership of the body pointers and will be the one to destroy them
	void AddBody(body* body);
	void AddBodies(const std::vector<body*>& body);

	void ToggleFluidGravity();
	void ToggleBodyGravity();
	void ToggleWind();

	// Do an explicit Euler time integration step
	void ExplicitEulerStep(float dt);

	// Removes all particles
	void Clear();

	std::vector<Particle*>&	GetParticles();
	std::vector<body*>&	GetBodies();
	body* movingBody;

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

	AABoundingBox			boundingBox;	// The bounding box in which the particles should reside
	std::vector<Particle*>	particles;		// These particles represent the fluid
	std::vector<body*>		bodies;			// The bodies in the simulation
	bool					fluidgravity;	// True if gravity force is to be applied on the fluid
	bool					bodygravity;	// True if gravity force is to be applied on the bodies
	bool					wind;			// True if wind force is to be applied
};