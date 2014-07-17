#pragma once

#include <glm/glm.hpp>
#include <glload/gl_3_3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// super for rigid bodies
class Body {
public:
	virtual glm::vec3 GetAngularVelocity(glm::vec3 contactpoint) = 0;
	virtual glm::vec3 GetVelocity() = 0;
	virtual glm::vec3 AbsoluteContactPoint(glm::vec3& relposition) = 0;
	virtual bool collision(const glm::vec3& position, const glm::vec3& newposition, glm::vec3& contactPoint, float& penDepth, glm::vec3& normal) = 0;


	glm::vec3	center;
	glm::vec3	velocity;
	glm::vec3	forceAccum;		// Force accumulator
	glm::vec3	rotation;
	glm::vec3	omega;
	float		mass;
};