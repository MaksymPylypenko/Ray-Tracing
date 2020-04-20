#include "model.h"

bool Plane::isHit(Ray ray) {

	float dotND = dot(normal, ray.direction);

	if (dotND < 0) {
		rayLen = dot(normal, center - ray.origin) / dotND;	
		if (rayLen > ray.minLen && rayLen < ray.maxLen) {
			return true;
		}		
	}

	return false;
}


void Plane::alignTextureAxes() {
	// This caused issues with a vertical plane on scene [c]...
	// axisU = normalize(glm::vec3(normal.y, normal.z, -normal.x));

	// Instead, setting [axisU] to the longest axis
	glm::vec3 a = cross(normal, glm::vec3(1, 0, 0));
	glm::vec3 b = cross(normal, glm::vec3(0, 1, 0));
	glm::vec3 c = cross(normal, glm::vec3(0, 0, 1));

	// Note, the dot product of a vector with itself is the square of its magnitude
	axisU = dot(a, a) > dot(b, b) ? a : b;
	axisU = normalize(dot(axisU, axisU) > dot(c, c) ? axisU : c);
	axisV = normalize(cross(normal, axisU));
}


void Plane::applyTexture(glm::vec3 hitPos) {
	float u = dot(hitPos, axisU);
	float v = dot(hitPos, axisV);	
	material->Ka = texture->getPixel(u, v);
}


void Plane::debug() {
	printf("Plane HIT %s @ RayLen = %f, \n", inside ? "from [Inside]" : "from [Outside]", rayLen);
}