#include "model.h"
#include "../common.h"


bool Object::isHit(Ray ray) {
	/// Abstract
	return false;
}


void Object::applyTexture(glm::vec3 hitPos) {
	/// Abstract
}


void Object::debug() {	
	printf("Abstract");	
}


