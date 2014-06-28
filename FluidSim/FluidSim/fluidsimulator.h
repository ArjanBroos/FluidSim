#pragma once

#include "particle.h"
#include "Body.h"
#include "Sphere.h"
#include <vector>
#include "boundingbox.h"
#include <iostream>

// Simulates fluids using particles
class FluidSimulator {
public:
	FluidSimulator(const AABoundingBox& boundingBox);
	~FluidSimulator();

	// This class takes ownership of the particle pointers and will be the one to destroy them
	void AddParticle(Particle* particle);
	void AddParticles(const std::vector<Particle*>& particles);

	// This class takes ownership of the body pointers and will be the one to destroy them
	void AddBody(Body* body);
	void AddBodies(const std::vector<Body*>& bodies);

	void ToggleFluidGravity();
	void ToggleBodyGravity();
	void ToggleWind();
	void ToggleSurfaceTension();
	void ToggleUseOctree();

	// Do an explicit Euler time integration step
	void ExplicitEulerStep(float dt);

	// Removes all particles
	void Clear();

	std::vector<Particle*>&	GetParticles();
	std::vector<Body*>&	GetBodies();
	AABoundingBox& GetBoundingBox() { return boundingBox; }
	Body* movingBody;

	bool isWind();
	bool isGravity();
	bool isSurfaceTension();
	bool isUseOctree();

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

	void		initOctree();
	void		calculateOctree();
	void		GetParticlesClose(Particle* pi, std::vector<Particle*>& particles);
	void		clearOctree();	// Frees memory from octree

	AABoundingBox			boundingBox;	// The bounding box in which the particles should reside
	std::vector<Particle*>	particles;		// These particles represent the fluid
	bool					gravity;		// True if gravity force is to be applied
	std::vector<Body*>		bodies;			// The bodies in the simulation
	bool					fluidgravity;	// True if gravity force is to be applied on the fluid
	bool					bodygravity;	// True if gravity force is to be applied on the bodies
	std::vector<std::vector<Particle*>>	octree; // octree for detecting particles close to one another
	bool					wind;			// True if wind force is to be applied
	bool					surfaceTension;	// True if surface tension force is to be applied
	
	std::vector< std::vector< Particle* > >	octree; // octree for detecting particles close to one another
	int						d1,d2,d3;		// dimensions of octree (including extra (empty) space on each side)
	float						c1,c2,c3;		// dimensions of octree (including extra (empty) space on each side)
	bool					useOctree;		// Use octree, else all particles are looped.
};