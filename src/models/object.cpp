#include "model.h"
#include "../common.h"


bool Object::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {
	/// Abstract
	return false;
}


void Object::applyTexture(glm::vec3 hitPos) {
	/// Abstract
}


void Object::debug() {	
	printf("Abstract");	
}


