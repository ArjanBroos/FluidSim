#pragma once

#include <glm/glm.hpp>

// Represents a particle in our fluid simulation
class Particle {
public:
	Particle();
	Particle(const glm::vec3& position);
	Particle(const glm::vec3& position, const glm::vec3& velocity);
	Particle(const glm::vec3& position, const glm::vec3& velocity, float mass);
	Particle(const glm::vec3& position, const glm::vec3& velocity, float mass, float restDensity);

	glm::vec3	position;
	glm::vec3	velocity;
	glm::vec3	forceAccum;		// Force accumulator
	float		mass;
	float		density;
	float		restDensity;
	float		pressure;
};