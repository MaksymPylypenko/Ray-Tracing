#include "model.h"


bool Sphere::isHit(Ray ray) {
		
	glm::vec3 L = ray.origin - center;
	float a = dot(ray.direction, ray.direction);
	float b = dot(ray.direction, L);
	float discriminant = b*b + radius * radius - a * dot(L, L);

	if (discriminant < 0) {
		return false;
	}

	float sqrtDisc = sqrt(discriminant);
	float rest = dot(-ray.direction, L) / a;

	if (discriminant > ray.minLen) {
		float t0 = rest + sqrtDisc / dot(ray.direction, ray.direction);
		float t1 = rest - sqrtDisc / dot(ray.direction, ray.direction);
	
		if (t0 > t1) {
			std::swap(t0, t1); // make sure t0 is the smallest
		}

		inside = false;
		if (t0 < ray.minLen) {
			//printf("\nT0 = %f, T1 = %f\n", t0, t1);
			t0 = t1;
			// ray might still hit from inside
			if (t0 < ray.minLen) {
				// t0 & t1 are before hitPos
				return false;
			} 
			else {
				inside = true;
			}			
		}
		rayLen = t0;
	}
	else {  // discriminant == 0 
		rayLen = rest;
	}

	if (rayLen < ray.minLen || rayLen > ray.maxLen) {
		//inside = false;
		return false;
	}
	
	normal = normalize((ray.origin + ray.direction * rayLen) - center);
	if (inside) {
		normal = -normal;
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
	printf("Sphere HIT %s @ RayLen = %f, \n", inside ? "from [Inside]" : "from [Outside]", rayLen);
}