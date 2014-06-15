#pragma once

#include <glm/glm.hpp>

glm::vec3	abs(const glm::vec3& v);
float		min(float s1, float s2);
glm::vec3	min(const glm::vec3& v1, const glm::vec3& v2);
float		max(float s1, float s2);
float		max(const glm::vec3& v);
glm::vec3	max(const glm::vec3& v1, const glm::vec3& v2);
float		sgn(float s);
glm::vec3	sgn(const glm::vec3& v);