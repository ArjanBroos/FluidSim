#pragma once

#include <glm/glm.hpp>
#include "body.h"

// rigid body box
class BoxRotating : public Body {
public:
	BoxRotating(glm::vec3 pos, glm::vec3 size, float m);

	glm::vec3 GetAngularVelocity(glm::vec3 contactpoint);
	glm::vec3 GetVelocity();
	glm::vec3 AbsoluteContactPoint(glm::vec3& relposition);

	bool collision(const glm::vec3& position, const glm::vec3& displacement, glm::vec3& contactPoint, float& penDepth, glm::vec3& normal);

	glm::vec3 size;
	
};
