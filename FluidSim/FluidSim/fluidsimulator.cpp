#include "fluidsimulator.h"

#include <algorithm>

FluidSimulator::~FluidSimulator() {
	for (auto p = particles.begin(); p != particles.end(); p++)
		delete *p;
	particles.clear();
}

// This class takes ownership of the particle pointers and will be the one to destroy them
void FluidSimulator::AddParticle(Particle* particle) {
	particles.push_back(particle);
}

void FluidSimulator::AddParticles(const std::vector<Particle*>& particles) {
	for (auto p = particles.begin(); p != particles.end(); p++)
		this->particles.push_back(*p);
}

std::vector<Particle*>&	FluidSimulator::GetParticles() {
	return particles;
}