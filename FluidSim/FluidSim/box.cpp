#include "box.h"
#include "util.h"
#include <glload/gl_3_3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



Box::Box(glm::vec3 pos, glm::vec3 size, float m){
	center = pos;
	this->size = size;
	mass = m;
}

glm::vec3 Box::GetAngularVelocity(glm::vec3 contactpoint){
	return glm::vec3(0.f,0.f,0.f); 
}

glm::vec3 Box::GetVelocity(){
	return velocity; 
}



bool Box::collision(const glm::vec3& position, const glm::vec3& displacement, glm::vec3& contactPoint, float& penDepth, glm::vec3& normal){
	float fLow = 0;
	float fHi = 1;
	int dir = -1;
	for (int i = 0; i < 3;i++){
		float newLow = ((center[i] - 0.5*size[i]) - position[i]) / (displacement[i]);
		float newHi = ((center[i] + 0.5*size[i]) - position[i]) / (displacement[i]);

		if (newLow>newHi){
			float temp = newLow;
			newLow = newHi;
			newHi = temp;
		}

		if (newHi<fLow || newLow>fHi){
			return false;
		}
		if (newLow>fLow){
			fLow = newLow;
			dir = i;
		}
		fHi = max(fHi, newHi);

		if (fLow>fHi){
			return false;
		}
	}
	//printf("%f\t%f\n",fLow,fHi);

	if (fLow<fHi && fLow<1 && fHi>0 && dir!=-1){
		//printf("%f\t%d\n",fLow,dir);
		contactPoint = (position + displacement*fLow) - center;
		penDepth = glm::length(displacement*(fHi-fLow));
		contactPoint = glm::vec3(0.f,0.f,0.f);
		if (position[dir]>center[dir]){
			contactPoint[dir] = 1;
		}
		else{
			contactPoint[dir] = -1;
		}
		return true;
	}
	return false;
}


glm::vec3 Box::AbsoluteContactPoint(glm::vec3& relposition){
	return center + relposition;

}