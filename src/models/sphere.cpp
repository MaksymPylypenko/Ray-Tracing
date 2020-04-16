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

	if (texture) {
		checkersTexture(rayOrigin + rayDir * rayLen);
	}
	

	return true;
}


void Sphere::checkersTexture(glm::vec3 hitPos) {
	// @TODO: decouple the texture thing..
	// Math: https://people.cs.clemson.edu/~dhouse/courses/405/notes/texture-maps.pdf

	glm::vec3 point = hitPos - center;

	// polar angle
	float phi = acosf(point.y / radius);

	// azimuthal angle
	float theta = atan2f(point.x, point.z);

	// The direction and proportion is not correct when reading texture from a regular bmp file!
	// @TODO See this for improvement http://www.raytracerchallenge.com/bonus/texture-mapping.html
	float u = theta / 3.1415926f;
	float v = phi / 3.1415926f;

	bool white = (int(round(u * scale)) + int(round(v * scale))) % 2 == 0;

	glm::vec3 colour;

	// Experimenting with bump mapping
	// @TODO make look better...
	/*normal = glm::vec4(normal, 1) * glm::rotate(glm::mat4(),
		glm::radians((float)(rand() % 40 - 20)),
		glm::vec3((rand() % 2), (rand() % 2), (rand() % 2)));*/

	if (white) {
		colour = glm::vec3(1.0, 1.0, 1.0);
	}
	else {
		colour = glm::vec3(0.1, 0.1, 0.1);
	}
	material->Ka = colour;
}

void Sphere::debug() {
	printf("Sphere @ RayLen = %f\n", rayLen);
}