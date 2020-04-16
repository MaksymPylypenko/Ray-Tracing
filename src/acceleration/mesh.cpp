#include "acceleration.h"

void Mesh::findBounds() {
	// init slabs to the first point in the mesh   

	min = triangles[0]->points[0];
	max = triangles[0]->points[0];
	for (Triangle* triangle : triangles) {
		for (glm::vec3 point : triangle->points) {
			for (int i = 0; i < 3; i++) {
				if (point[i] < min[i]) {
					min[i] = point[i];
				}
				else if (point[i] > max[i]) {
					max[i] = point[i];
				}
			}
		}
	}
	center = (min + max) / 2.0f;
}


bool Mesh::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {

	rayLen = maxRayLen;

	for (Triangle* triangle : triangles) {
		if (triangle->isHit(rayOrigin, rayDir, minRayLen, maxRayLen, inside)) {
			if (triangle->rayLen < rayLen) {					
				rayLen = triangle->rayLen;
				normal = triangle->normal;
				material = triangle->material;
			}
		}
	}
	if (rayLen != maxRayLen) {
		return true;
	}
	return false;
}


void Mesh::debug() {
	printf("Mesh @ RayLen = %f\n", rayLen);
}


/// Acceleration 

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

		glm::vec3 center = triangle->getBarycenter();
		//printf("Barycenter = %f, %f, %f", triCenter.x, triCenter.y, triCenter.z);

		triangle->nodeID = 0;
		if (center.x > mesh->center.x) {
			triangle->nodeID += 1;
		}
		if (center.y > mesh->center.y) {
			triangle->nodeID += 2;
		}
		if (center.z > mesh->center.z) {
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


bool MeshHierarchy::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {
		
	if(!isHitBounds(mesh->min, mesh->max, rayOrigin, rayDir, minRayLen, maxRayLen)){
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
	// init slabs to the first point in the mesh   

	min = mesh->min;
	max = mesh->max;
	center = mesh->center;	
}



// Transformations

void Mesh::translate(glm::vec3 vector) {
	for (Triangle* triangle : triangles) {
		for (int i = 0; i < 3; i++) {
			triangle->points[i] += vector;
		}
	}
}


void Mesh::scale(float scale) {
	for (Triangle* triangle : triangles) {
		for (int i = 0; i < 3; i++) {
			glm::vec3 currPos = triangle->points[i] - center;
			glm::vec3 newPos = currPos * scale;
			triangle->points[i] += newPos - currPos;
		}
	}
}


void Mesh::addQuaternion(glm::vec3 axis, float angle) {
	q = glm::rotate(q, angle, normalize(axis));
}

void Mesh::rotate() {
	for (Triangle* triangle : triangles) {
		for (int i = 0; i < 3; i++) {
			glm::vec3 currPos = triangle->points[i] - center;
			glm::vec3 newPos = currPos * mat3_cast(q);
			triangle->points[i] += newPos - currPos;
		}
	}
}
