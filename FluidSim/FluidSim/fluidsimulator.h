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

	std::vector<Particle*>&	GetParticles();

private:
	std::vector<Particle*>	particles;	// These particles represent the fluid
};