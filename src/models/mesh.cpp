#include "model.h"


bool Mesh::isHit(Ray ray, Hit& hit) {
	
	Hit curr = Hit();	

	if (ray.closest) {
		for (Triangle* triangle : triangles) {
			if (triangle->isHit(ray, curr)) {
				if (curr.rayLen < hit.rayLen) {
					hit = curr;
				}
			}
		}

		if (hit.object != nullptr) {
			return true;
		}
	}
	else {
		for (Triangle* triangle : triangles) {
			if (triangle->isHit(ray, curr)) {
				return true;
			}
		}
	}
	
	return false;
}


void Mesh::resetOrigin() {
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



// Transformations

void Mesh::resetBarycenters() {
	for (Triangle* triangle : triangles) {
		triangle->setBarycenter();
	}
}

void Mesh::resetNormals() {
	for (Triangle* triangle : triangles) {
		triangle->setNormal();
	}
}

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