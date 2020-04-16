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


///////////////////


bool BVH::build(BoundingVolume* newBV, int threshold) {

	bv = newBV;	
	bv->findBounds();

	// BVH contains a minimum number of objects, this is a base case.
	if (bv->objects.size() <= threshold) { // @TODO change to 1
		isLeave = true;
		return true;
	}
	

	// Else, classify each object to 1 of the 8 nodes
	int nodePointsNum[8] = { 0 };
	//printf("Current BV:\n");
	for (Object* obj : bv->objects) {
		glm::vec3 center = obj->center;

		obj->nodeID = 0;
		if (center.x > bv->center.x) {
			obj->nodeID += 1;
		}
		if (center.y > bv->center.y) {
			obj->nodeID += 2;
		}
		if (center.z > bv->center.z) {
			obj->nodeID += 4;
		}

		// Remember which nodes have triangles. This allows to ignore empty nodes
		nodePointsNum[obj->nodeID]++;
	}

	for (int i = 0; i < 8; i++) {
		if (nodePointsNum[i] != 0) {

			children[i] = new BVH();

			std::vector<Object*> subset;

			for (Object* obj : bv->objects) {
				if (obj->nodeID == i) {
					subset.push_back(obj);
				}
			}

			BoundingVolume* subBV = new BoundingVolume();
			subBV->objects = subset;
			children[i]->build(subBV, threshold);
		}
	}
	return true;
}


bool BVH::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {

	if (!isHitBounds(bv->min, bv->max, rayOrigin, rayDir, minRayLen, maxRayLen)) {
		return false;
	}

	if (isLeave) { // Checking atomic objects
		if (bv->isHit(rayOrigin, rayDir, minRayLen, maxRayLen, inside)) {
			closest = bv->closest;
			return true;
		}
		else {
			return false;
		}
	}
	else { // Checking the next round of Bounding Volumes	
		closest = new Object();
		closest->rayLen = maxRayLen;

		for (BVH* child : children) {
			if (child) {
				if (child->isHit(rayOrigin, rayDir, minRayLen, maxRayLen, inside)) {
					if (child->closest->rayLen < closest->rayLen) { // looking for the closest hit
						closest = child->closest;
					}
				}
			}
		}
		if (closest->rayLen != maxRayLen) {
			return true;
		}
	}
	
	return false;
}


