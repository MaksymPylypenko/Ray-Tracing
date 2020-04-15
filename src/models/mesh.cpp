#include "model.h"


void Mesh::findSlabs() {
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
}


bool Mesh::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {

	rayLen = maxRayLen;

	for (Triangle* triangle : triangles) {
		if (triangle->isHit(rayOrigin, rayDir, minRayLen, maxRayLen, inside)) {
			if (triangle->rayLen < rayLen) {
				rayLen = triangle->rayLen;
				material = triangle->material;
				normal = triangle->normal;
				rayLen = triangle->rayLen;
			}
		}
	}
	if (rayLen != maxRayLen) {
		return true;
	}
	return false;
}

void Mesh::translate(glm::vec3 vector) {
	for (Triangle* triangle : triangles) {
		for (int i = 0; i < 3; i++) {
			triangle->points[i] += vector;
		}
	}
}

void Mesh::resetOrigin() {
	findSlabs();
	origin = (min + max) / 2.0f;
}

void Mesh::scale(float scale) {
	for (Triangle* triangle : triangles) {
		for (int i = 0; i < 3; i++) {
			glm::vec3 currPos = triangle->points[i] - origin;
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
			glm::vec3 currPos = triangle->points[i] - origin;
			glm::vec3 newPos = currPos * mat3_cast(q);
			triangle->points[i] += newPos - currPos;
		}
	}
}


void Mesh::debug() {
	printf("Mesh @ RayLen = %f\n", rayLen);
}
