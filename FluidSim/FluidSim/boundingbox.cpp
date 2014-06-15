#include "boundingbox.h"
#include "util.h"

AABoundingBox::AABoundingBox() :
	left(-100.f), bottom(-100.f), back(-100.f),
	right(100.f), top(100.f), front(100.f) {
}

AABoundingBox::AABoundingBox(const glm::vec3& center, float size) :
	left(center.x - size/2.f), bottom(center.y - size/2.f), back(center.z - size/2.f),
	right(center.x + size/2.f), top(center.y + size/2.f), front(center.z + size/2.f) {
}

// Checks if position is outside the bounding box
// If so, it sets the following output parameters:
// contactPoint - point of intersection
// penDepth - penetration depth
// normal - normal of the boundingbox at the point of intersection
bool AABoundingBox::Outside(const glm::vec3& position, glm::vec3& contactPoint, float& penDepth, glm::vec3& normal) const {
	glm::vec3 center((left + right) / 2.f, (bottom + top) / 2.f, (back + front) / 2.f);
	glm::vec3 localPos = position - center;
	glm::vec3 ext(right - center.x, top - center.y, front - center.z);

	float fbox = max(abs(localPos) - ext);
	// If outside of the bounding box
	if (fbox > 0.f) {
		glm::vec3 cpLocal = min(ext, max(-ext, localPos));
		contactPoint = center + cpLocal;
		penDepth = glm::length(cpLocal - localPos);
		normal = sgn(cpLocal - localPos) / glm::length(sgn(cpLocal - localPos));
		return true;
	}

	return false;
}