#pragma once

#include <glm/glm.hpp>
#include "body.h"

// Axis aligned bounding box for the particles to stay within
class Sphere : public Body {
public:
	Sphere(glm::vec3 pos, float size);

	glm::vec3 GetAngularVelocity(glm::vec3 contactpoint);
	glm::vec3 GetVelocity();
	glm::vec3 ContactNormal();
	glm::vec3 AbsoluteContactPoint(glm::vec3& relposition);

	bool Collision(const glm::vec3& parposition, glm::vec3& contactPoint, float& penDepth, glm::vec3& normal);

	float size;
};
