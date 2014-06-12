#pragma once

#include <glm/glm.hpp>

// Axis aligned bounding box for the particles to stay within
class AABoundingBox {
public:
	AABoundingBox();
	AABoundingBox(const glm::vec3& center, float size);

	// Checks if position is outside the bounding box
	// If so, it sets contactPoint to the point of intersection
	// and normal to the normal of the box at the point of intersection
	bool Outside(const glm::vec3& position, glm::vec3& contactPoint, glm::vec3& normal) const;

	// Axis planes
	float left;
	float right;
	float bottom;
	float top;
	float back;
	float front;

private:
	glm::vec3	abs(const glm::vec3& v) const;
	float		min(float s1, float s2) const;
	glm::vec3	min(const glm::vec3& v1, const glm::vec3& v2) const;
	float		max(float s1, float s2) const;
	float		max(const glm::vec3& v) const;
	glm::vec3	max(const glm::vec3& v1, const glm::vec3& v2) const;
	float		sgn(float s) const;
	glm::vec3	sgn(const glm::vec3& v) const;
};