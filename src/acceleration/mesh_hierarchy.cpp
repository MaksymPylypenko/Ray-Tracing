#include "acceleration.h"


bool MeshHierarchy::build(Mesh* newMesh, int threshold, int maxDepth, int currDepth) {

	mesh = newMesh;
	mesh->findBounds();

	// Mesh contains a minimum number of objects, this is a base case.
	if (mesh->triangles.size() <= threshold || currDepth >= maxDepth) {
		//printf("Leave node has this number of triangles = %d\n", mesh->triangles.size());
		isLeave = true;
		return true;
	}

	// Else, classify each triangle to 1 of the 8 nodes
	int nodePointsNum[8] = { 0 };

	
	for (Triangle* triangle : mesh->triangles) {
		triangle->nodeID = 0;
		if (triangle->center.x > mesh->center.x) {
			triangle->nodeID += 1;
		}
		if (triangle->center.y > mesh->center.y) {
			triangle->nodeID += 2;
		}
		if (triangle->center.z > mesh->center.z) {
			triangle->nodeID += 4;
		}

		// Remember which nodes have triangles. This allows to ignore empty nodes
		nodePointsNum[triangle->nodeID]++;
	}

	for (int i = 0; i < 8; i++) {
		if (nodePointsNum[i] != 0) {
			//printf("Node %d, number of triangles = %d\n", i, nodePointsNum[i]);

			children[i] = new MeshHierarchy();

			std::vector<Triangle*> subset;

			for (Triangle* triangle : mesh->triangles) {
				if (triangle->nodeID == i) {
					subset.push_back(triangle);
				}
			}

			Mesh* subMesh = new Mesh();
			subMesh->triangles = subset;
			children[i]->build(subMesh, threshold, maxDepth, currDepth + 1);
		}
	}
	return true;
}


bool MeshHierarchy::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, 
	float maxRayLen, bool inside) {

	// @TODO add shadow ray hit...

	if (!isHitBounds(mesh->min, mesh->max, rayOrigin, rayDir, minRayLen, maxRayLen)) {
		return false;
	}

	if (isLeave) { // Checking each triangle on the mesh			
		if (mesh->isHit(rayOrigin, rayDir, minRayLen, maxRayLen, inside)) {
			rayLen = mesh->rayLen;
			normal = mesh->normal;
			material = mesh->material;
			return true;
		}
		else {
			return false;
		}
	}
	else { // Checking the next round of Bounding Volumes	
		rayLen = maxRayLen;

		for (MeshHierarchy* child : children) {
			if (child) {
				if (child->isHit(rayOrigin, rayDir, minRayLen, maxRayLen, inside)) {
					if (child->rayLen < rayLen) { // looking for the closest hit
						rayLen = child->rayLen;
						normal = child->normal;
						material = child->material;
					}
				}
			}
		}
		if (rayLen != maxRayLen) {
			return true;
		}
	}

	return false;
}

void MeshHierarchy::debug() {
	printf("MeshHierarchy @ RayLen = %f\n", rayLen);
}


void MeshHierarchy::findBounds() {
	min = mesh->min;
	max = mesh->max;
	center = mesh->center;
}
