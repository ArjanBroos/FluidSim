#pragma once

#include <glm/glm.hpp>
#include <glload/gl_3_3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Axis aligned bounding box for the particles to stay within
class body {
public:
	virtual void draw(GLuint mvpMatrixUniform,
		GLuint modelViewMatrixUniform,
		GLuint normalMatrixUniform,
		glm::mat4 modelMatrix,
		glm::mat4 viewMatrix,
		glm::mat4 projectionMatrix,
		glm::mat4 mvpMatrix,
		glm::mat4 modelViewMatrix,
		glm::mat4 normalMatrix) = 0;
	virtual glm::vec3 GetAngularVelocity(glm::vec3 contactpoint) = 0;
	virtual glm::vec3 GetVelocity() = 0;
	virtual glm::vec3 contactNormal() = 0;
	virtual bool collision(const glm::vec3& parposition, glm::vec3& contactPoint, float& penDepth, glm::vec3& normal) = 0;

	glm::vec3	position;
	glm::vec3	velocity;
	glm::vec3	forceAccum;		// Force accumulator
	float		mass;
};