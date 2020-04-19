#include "model.h"

bool Sphere::isHit(Ray ray) {

	float bSq = pow(dot(ray.direction, ray.origin - center), 2);
	float FourAC = dot(ray.direction, ray.direction) * dot(ray.origin - center, ray.origin - center) - radius * radius;
	float discriminant = bSq - FourAC;

	if (discriminant < 0) {
		return false;
	}

	float sqrtDisc = sqrt(discriminant);
	float rest = dot(-ray.direction, ray.origin - center) / dot(ray.direction, ray.direction);

	if (discriminant > 0) {
		float t1 = rest + sqrtDisc / dot(ray.direction, ray.direction);
		float t2 = rest - sqrtDisc / dot(ray.direction, ray.direction);
		if (ray.isInside) {
			t1 > t2 ? rayLen = t1 : rayLen = t2;
		}
		else {
			t1 < t2 ? rayLen = t1 : rayLen = t2;
		}
	}
	else {  // discriminant < 0 
		rayLen = rest;
	}

	if (rayLen < ray.minLen || rayLen > ray.maxLen) {
		return false;
	}

	normal = normalize((ray.origin + ray.direction * rayLen) - center);
	if (ray.isInside) {
		normal = -normal;
	}

	if (texture->mode != TextureMode::none) {
		applyTexture(ray.origin + ray.direction * rayLen);
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