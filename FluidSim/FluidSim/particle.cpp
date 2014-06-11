#include "particle.h"

Particle::Particle() :
	position(glm::vec3(0.f, 0.f, 0.f)),
	velocity(glm::vec3(0.f, 0.f, 0.f)),
	forceAccum(glm::vec3(0.f, 0.f, 0.f)) {
}

Particle::Particle(const glm::vec3& position) :
	position(position),
	velocity(glm::vec3(0.f, 0.f, 0.f)),
	forceAccum(glm::vec3(0.f, 0.f, 0.f)) {
}
Particle::Particle(float x, float y, float z) :
	position(glm::vec3(x, y, z)),
	velocity(glm::vec3(0.f, 0.f, 0.f)),
	forceAccum(glm::vec3(0.f, 0.f, 0.f)) {
}

Particle::Particle(const glm::vec3& position, const glm::vec3& velocity) :
	position(position),
	velocity(velocity),
	forceAccum(glm::vec3(0.f, 0.f, 0.f)) {
}

Particle::Particle(float px, float py, float pz, float vx, float vy, float vz) :
	position(glm::vec3(px, py, pz)),
	velocity(glm::vec3(vx, vy, vz)),
	forceAccum(glm::vec3(0.f, 0.f, 0.f)) {
}

glm::vec3 Particle::GetPosition() const {
	return position;
}

glm::vec3 Particle::GetVelocity() const {
	return velocity;
}

glm::vec3 Particle::GetForceAccum() const {
	return forceAccum;
}