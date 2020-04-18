#include "model.h"

bool Sphere::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {

	float bSq = pow(dot(rayDir, rayOrigin - center), 2);
	float FourAC = dot(rayDir, rayDir) * dot(rayOrigin - center, rayOrigin - center) - radius * radius;
	float discriminant = bSq - FourAC;

	if (discriminant < 0) {
		return false;
	}

	float sqrtDisc = sqrt(discriminant);
	float rest = dot(-rayDir, rayOrigin - center) / dot(rayDir, rayDir);

	if (discriminant > 0) {
		float t1 = rest + sqrtDisc / dot(rayDir, rayDir);
		float t2 = rest - sqrtDisc / dot(rayDir, rayDir);
		if (inside) {
			t1 > t2 ? rayLen = t1 : rayLen = t2;
		}
		else {
			t1 < t2 ? rayLen = t1 : rayLen = t2;
		}
	}
	else {  // discriminant < 0 
		rayLen = rest;
	}

	if (rayLen < minRayLen || rayLen > maxRayLen) {
		return false;
	}

	normal = normalize((rayOrigin + rayDir * rayLen) - center);
	if (inside) {
		normal = -normal;
	}

	if (texture->mode != TextureMode::none) {
		applyTexture(rayOrigin + rayDir * rayLen);
	}
	

	// Experimenting with bump mapping
	//normal = glm::vec4(normal, 1) * glm::rotate(glm::mat4(),
	//glm::radians((float)(rand() % 160 - 80)),
	//glm::vec3((rand() % 2), (rand() % 2), (rand() % 2)));

	return true;
}


void Sphere::applyTexture(glm::vec3 hitPos) {
	// Resource used: 
	// 1. https://people.cs.clemson.edu/~dhouse/courses/405/notes/texture-maps.pdf
	// 2. http://www.raytracerchallenge.com/bonus/texture-mapping.html

	glm::vec3 point = hitPos - center;
	float phi = acosf(point.y / radius);
	float theta = atan2f(point.x, point.z);	
	float u = theta / 6.2831852f;
	float v = phi / 3.1415926f;
	material->Ka = texture->getPixel(u, v);
}

void Sphere::debug() {
	printf("Sphere @ RayLen = %f\n", rayLen);
}