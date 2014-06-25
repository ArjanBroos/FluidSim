#include "sphere.h"
#include "util.h"
#include <glload/gl_3_3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Sphere::Sphere(glm::vec3 pos, float size){
	position = pos;
	this->size = size;
}

glm::vec3 Sphere::GetAngularVelocity(glm::vec3 contactpoint){
	return glm::vec3(0.f,0.f,0.f); 
}

glm::vec3 Sphere::GetVelocity(){
	return velocity; 
}

glm::vec3 Sphere::ContactNormal(){
	return glm::vec3(0.f, 0.f, 0.f);
}

bool Sphere::Collision(const glm::vec3& parposition, glm::vec3& contactPoint, float& penDepth, glm::vec3& normal){
	penDepth = size - sqrt((position[0] - parposition[0])*(position[0] - parposition[0]) + (position[1] - parposition[1])*(position[1] - parposition[1]) + (position[2] - parposition[2])*(position[2] - parposition[2]));
	if ( penDepth > 0) {
		normal = (parposition - position) / glm::length((parposition - position));
		contactPoint = normal*size;
		return true;
	}

	return false;
}

glm::vec3 Sphere::AbsoluteContactPoint(glm::vec3& relposition){
	return position + relposition;

}