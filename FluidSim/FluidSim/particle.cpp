#include "particle.h"

const glm::vec3 standardPosition(0.f, 0.f, 0.f);
const glm::vec3 standardVelocity(0.f, 0.f, 0.f);
const glm::vec3 standardForceAccum(0.f, 0.f, 0.f);
const float standardMass = 1.f;
const float standardDensity = 0.f;
const float standardRestDensity = 1.f;
const float standardPressure = 0.f;
const int standardHash = -1;

Particle::Particle() :
	position(standardPosition),
	velocity(standardVelocity),
	forceAccum(standardForceAccum),
	mass(standardMass),
	density(standardDensity),
	restDensity(standardRestDensity),
	pressure(standardPressure),
	hashOctree(standardHash) {
}

Particle::Particle(const glm::vec3& position) :
	position(position),
	velocity(standardVelocity),
	forceAccum(standardForceAccum),
	mass(standardMass),
	density(standardDensity),
	restDensity(standardRestDensity),
	pressure(standardPressure),
	hashOctree(standardHash) {
}

Particle::Particle(const glm::vec3& position, const glm::vec3& velocity) :
	position(position),
	velocity(velocity),
	forceAccum(standardForceAccum),
	mass(standardMass),
	density(standardDensity),
	restDensity(standardRestDensity),
	pressure(standardPressure),
	hashOctree(standardHash) {
}

Particle::Particle(const glm::vec3& position, const glm::vec3& velocity, float mass) :
	position(position),
	velocity(velocity),
	forceAccum(standardForceAccum),
	mass(mass),
	density(standardDensity),
	restDensity(standardRestDensity),
	pressure(standardPressure),
	hashOctree(standardHash) {
}

Particle::Particle(const glm::vec3& position, const glm::vec3& velocity, float mass, float restDensity) :
	position(position),
	velocity(velocity),
	forceAccum(standardForceAccum),
	mass(mass),
	density(standardDensity),
	restDensity(restDensity),
	pressure(standardPressure),
	hashOctree(standardHash) {
}