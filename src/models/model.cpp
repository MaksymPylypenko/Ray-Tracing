#include "model.h"


bool Object::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {
	return false;
}

void Object::debug() {	
	printf("None");	
}


/// Acceleration 


bool BoundingVolume::build(Mesh* newMesh, int threshold, int maxDepth, int currDepth) {

	mesh = newMesh;
	mesh->resetOrigin();

	// Mesh contains a minimum number of objects, this is a base case.
	if (mesh->triangles.size() <= threshold || currDepth >= maxDepth)	{
		//printf("Leave node has this number of triangles = %d\n", mesh->triangles.size());
		isLeave = true;
		return true;
	}
	   
	// Else, classify each triangle to 1 of the 8 nodes
	int nodePointsNum[8] = { 0 };

	for (Triangle* triangle : mesh->triangles) {

		glm::vec3 center = triangle->getBarycenter();	
		//printf("Barycenter = %f, %f, %f", triCenter.x, triCenter.y, triCenter.z);

		triangle->nodeID = 0;
		if (center.x > mesh->origin.x) {
			triangle->nodeID += 1;
		}
		if (center.y > mesh->origin.y) {
			triangle->nodeID += 2;
		}
		if (center.z > mesh->origin.z) {
			triangle->nodeID += 4;
		}

		// Remember which nodes have triangles. This allows to ignore empty nodes
		nodePointsNum[triangle->nodeID]++;
	}
	   
	for (int i = 0; i < 8; i++)	{
		if (nodePointsNum[i]!=0) {
			//printf("Node %d, number of triangles = %d\n", i, nodePointsNum[i]);

			children[i] = new BoundingVolume();

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


bool BoundingVolume::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {
	
	// Optimization for AABB intersection test
	// https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525

	glm::vec3 invRayDir = 1.0f / rayDir;
	glm::vec3 t0 = (mesh->min - rayOrigin) * invRayDir;
	glm::vec3 t1 = (mesh->max - rayOrigin) * invRayDir;

	glm::vec3 tmin = glm::min(t0, t1);
	glm::vec3 tmax = glm::max(t0, t1);

	float lowest = glm::max(minRayLen, glm::max(tmin[0], glm::max(tmin[1], tmin[2])));
	float highest = glm::min(maxRayLen, glm::min(tmax[0], glm::min(tmax[1], tmax[2])));

	if (lowest <= highest) { // Bounding Volume was hit, checking children
		
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

			for (BoundingVolume* child : children) {
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
	}
	return false;
}

void BoundingVolume::debug() {
	printf("BoundingVolume @ RayLen = %f\n", rayLen);
}
