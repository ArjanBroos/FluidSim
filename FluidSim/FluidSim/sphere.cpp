#include "sphere.h"
#include "util.h"
#include <glload/gl_3_3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



Sphere::Sphere(glm::vec3 pos, float size, float m){
	center = pos;
	this->size = size;
	mass = m;
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


bool Sphere::collision(const glm::vec3& position, const glm::vec3& displacement, glm::vec3& contactPoint, float& penDepth, glm::vec3& normal){
	glm::vec3 pos = position - center;
	float A = displacement[0] * displacement[0] + displacement[1] * displacement[1] + displacement[2] * displacement[2];
	float B = 2*pos[0] * displacement[0] + 2* pos[1] * displacement[1] + 2* pos[2] * displacement[2];
	float C = pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2] - (size*size);
	float D = B*B - 4 * A * C;
	if (D<=0||A==0){
		if (A==0){
			printf("%d\n", rand());
		}
		return false;
	}
	float ans1 = (-B + sqrt(D)) / (2 * A);
	float ans2 = (-B - sqrt(D)) / (2 * A);
	float ans;

	if (0>ans1||ans1>1) {
		if (0>ans2 || ans2>1){
			return false;
		}
		else{
			ans = ans2;
		}
	}
	else{
		if (0>ans2 || ans2>1){
			ans = ans1;
		}
		else{
			ans = min(ans1,ans2);
		}
	}

	contactPoint = (pos + (displacement*ans));
	normal = contactPoint / size;
	return true;
}


glm::vec3 Sphere::AbsoluteContactPoint(glm::vec3& relposition){
	return center + relposition;

}