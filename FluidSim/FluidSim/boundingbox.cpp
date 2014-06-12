#include "boundingbox.h"

AABoundingBox::AABoundingBox() :
	left(-100.f), bottom(-100.f), back(-100.f),
	right(100.f), top(100.f), front(100.f) {
}

AABoundingBox::AABoundingBox(const glm::vec3& center, float size) :
	left(center.x - size/2.f), bottom(center.y - size/2.f), back(center.z - size/2.f),
	right(center.x + size/2.f), top(center.y + size/2.f), front(center.z + size/2.f) {
}

// Checks if position is outside the bounding box
// If so, it sets contactPoint to the point of intersection
// and normal to the normal of the box at the point of intersection
bool AABoundingBox::Outside(const glm::vec3& position, glm::vec3& contactPoint, glm::vec3& normal) const {
	glm::vec3 center((left + right) / 2.f, (bottom + top) / 2.f, (back + front) / 2.f);
	glm::vec3 localPos = position - center;
	glm::vec3 ext(right - center.x, top - center.y, front - center.z);

	float fbox = max(abs(localPos) - ext);
	// If outside of the bounding box
	if (fbox > 0.f) {
		glm::vec3 cpLocal = min(ext, max(-ext, localPos));
		contactPoint = center + cpLocal;
		normal = sgn(cpLocal - localPos) / glm::length(sgn(cpLocal - localPos));
		return true;
	}

	return false;
}

glm::vec3 AABoundingBox::abs(const glm::vec3& v) const {
	return glm::vec3(fabsf(v.x), fabsf(v.y), fabsf(v.z));
}

float AABoundingBox::min(float s1, float s2) const {
	return s1 < s2 ? s1 : s2;
}

glm::vec3 AABoundingBox::min(const glm::vec3& v1, const glm::vec3& v2) const {
	return glm::vec3(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z));
}

float AABoundingBox::max(float s1, float s2) const {
	return s1 > s2 ? s1 : s2;
}

float AABoundingBox::max(const glm::vec3& v) const {
	return v.x > v.y ? (v.x > v.z ? v.x : v.z) : (v.y > v.z ? v.y : v.z);
}

glm::vec3 AABoundingBox::max(const glm::vec3& v1, const glm::vec3& v2) const {
	return glm::vec3(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z));
}

float AABoundingBox::sgn(float s) const {
	if (s == 0.f) return s;
	else if (s > 0.f) return 1.f;
	else return -1.f;
}

glm::vec3 AABoundingBox::sgn(const glm::vec3& v) const {
	return glm::vec3(sgn(v.x), sgn(v.y), sgn(v.z));
}