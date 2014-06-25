#pragma once

#include <glm/glm.hpp>

// Axis aligned bounding box for the particles to stay within
class AABoundingBox {
public:
	AABoundingBox();
	AABoundingBox(const glm::vec3& center, float size);

	// Checks if position is outside the bounding box
	// If so, it sets the following output parameters:
	// contactPoint - point of intersection
	// penDepth - penetration depth
	// normal - normal of the boundingbox at the point of intersection
	bool Outside(const glm::vec3& position, glm::vec3& contactPoint, float& penDepth, glm::vec3& normal) const;
	bool Inside(const glm::vec3& position, glm::vec3& contactPoint, float& penDepth, glm::vec3& normal) const;

	// Axis planes
	float left;
	float right;
	float bottom;
	float top;
	float back;
	float front;

	glm::vec3   center;
	float		size;
};