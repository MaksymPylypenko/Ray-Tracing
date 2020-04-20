#include "model.h"
#include "../common.h"


bool Object::isHit(Ray ray, Hit& hit) {
	/// Abstract
	return false;
}


void Object::applyTexture(glm::vec3 hitPos) {
	/// Abstract
}


void Object::toString() {
	printf("Abstract");	
}


void debug(Ray& ray, Hit& hit) {
	hit.object->toString();
	printf(" HIT %s @ RayLen = %f, \n", 
		hit.inside ? "from [Inside]" : "from [Outside]", 
		hit.rayLen
	);
}