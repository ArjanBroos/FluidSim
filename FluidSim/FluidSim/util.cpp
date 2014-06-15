#include "util.h"

glm::vec3 abs(const glm::vec3& v) {
	return glm::vec3(fabsf(v.x), fabsf(v.y), fabsf(v.z));
}

float min(float s1, float s2) {
	return s1 < s2 ? s1 : s2;
}

glm::vec3 min(const glm::vec3& v1, const glm::vec3& v2) {
	return glm::vec3(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z));
}

float max(float s1, float s2) {
	return s1 > s2 ? s1 : s2;
}

float max(const glm::vec3& v) {
	return v.x > v.y ? (v.x > v.z ? v.x : v.z) : (v.y > v.z ? v.y : v.z);
}

glm::vec3 max(const glm::vec3& v1, const glm::vec3& v2) {
	return glm::vec3(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z));
}

float sgn(float s) {
	if (s == 0.f) return s;
	else if (s > 0.f) return 1.f;
	else return -1.f;
}

glm::vec3 sgn(const glm::vec3& v) {
	return glm::vec3(sgn(v.x), sgn(v.y), sgn(v.z));
}