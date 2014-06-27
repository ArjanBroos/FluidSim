#pragma once

#include <glm/glm.hpp>
#include "body.h"

// Axis aligned bounding box for the particles to stay within
class Sphere : public Body {
public:
	sphere(glm::vec3 pos, float size, float m);

	glm::vec3 GetAngularVelocity(glm::vec3 contactpoint);
	glm::vec3 GetVelocity();
	glm::vec3 ContactNormal();
	glm::vec3 AbsoluteContactPoint(glm::vec3& relposition);

	bool collision(const glm::vec3& position, const glm::vec3& displacement, glm::vec3& contactPoint, float& penDepth, glm::vec3& normal);

	float size;
};
