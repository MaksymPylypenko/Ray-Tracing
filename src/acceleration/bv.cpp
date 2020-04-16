#include "acceleration.h"


bool BoundingVolume::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir,
	float minRayLen, float maxRayLen, bool inside) {

	closest = new Object();
	closest->rayLen = maxRayLen;

	// traverse the objects	
	for (Object* object : objects) {
		if (object->isHit(rayOrigin, rayDir, minRayLen, maxRayLen, inside)) {
			if (object->rayLen < closest->rayLen) {
				closest = object;
			}
		}
	}

	if (closest->rayLen == maxRayLen) {
		return false;
	}

	return true;
}


void BoundingVolume::findBounds() {

	// init slabs to the first point in the mesh
	//objects[0]->findBounds();
	min = objects[0]->min;
	max = objects[0]->max;

	for (Object* obj : objects) {
		//obj->findBounds();
		for (int i = 0; i < 3; i++) {
			if (obj->min[i] < min[i]) {
				min[i] = obj->min[i];
			}
			else if (obj->max[i] > max[i]) {
				max[i] = obj->max[i];
			}
		}
	}
	center = (min + max) / 2.0f;
}