#pragma once

#include <glm/glm.hpp>

// Represents a particle in our fluid simulation
class Particle {
public:
	Particle();
	Particle(const glm::vec3& position);
	Particle(float x, float y, float z);
	Particle(const glm::vec3& position, const glm::vec3& velocity);
	Particle(float px, float py, float pz, float vx, float vy, float vz);

	glm::vec3 GetPosition() const;
	glm::vec3 GetVelocity() const;
	glm::vec3 GetForceAccum() const;	// Retrieve force accumulator

private:
	glm::vec3	position;
	glm::vec3	velocity;
	glm::vec3	forceAccum;		// Force accumulator
};