#pragma once

#include <glm/glm.hpp>
#include "Body.h"

// Axis aligned bounding box for the particles to stay within
class sphere : public body {
public:
	sphere(glm::vec3 pos, float size);

	void draw(GLuint mvpMatrixUniform,
		GLuint modelViewMatrixUniform,
		GLuint normalMatrixUniform,
		glm::mat4 modelMatrix,
		glm::mat4 viewMatrix,
		glm::mat4 projectionMatrix,
		glm::mat4 mvpMatrix,
		glm::mat4 modelViewMatrix,
		glm::mat4 normalMatrix);
	glm::vec3 GetAngularVelocity(glm::vec3 contactpoint);
	glm::vec3 GetVelocity();
	glm::vec3 contactNormal();
	glm::vec3 absoluteContactPoint(glm::vec3& relposition);

	bool collision(const glm::vec3& parposition, glm::vec3& contactPoint, float& penDepth, glm::vec3& normal);

	float size;
};
