#include "model.h"
#include "../common.h"


bool Object::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {
	return false;
}

void Object::debug() {	
	printf("None");	
}


void Object::findBounds() {
	///
}

bool isHitBounds(glm::vec3 min, glm::vec3 max, glm::vec3 rayOrigin, glm::vec3 rayDir,
	float minRayLen, float maxRayLen) {

	// Optimization for AABB intersection test
	// https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525

	glm::vec3 invRayDir = 1.0f / rayDir;
	glm::vec3 t0 = (min - rayOrigin) * invRayDir;
	glm::vec3 t1 = (max - rayOrigin) * invRayDir;

	glm::vec3 tmin = glm::min(t0, t1);
	glm::vec3 tmax = glm::max(t0, t1);

	float lowest = glm::max(minRayLen, glm::max(tmin[0], glm::max(tmin[1], tmin[2])));
	float highest = glm::min(maxRayLen, glm::min(tmax[0], glm::min(tmax[1], tmax[2])));

	return lowest <= highest;
}