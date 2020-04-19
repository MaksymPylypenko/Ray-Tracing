#include "ray.h"


void Ray::reflect(glm::vec3 hitPos, glm::vec3 N, glm::vec3 V) {
	origin = hitPos;
	direction = normalize(2 * dot(N, V) * N - V);
}


bool Ray::refract(glm::vec3 hitPos, glm::vec3 N, float eta, bool inside) {

	origin = hitPos;

	float dotIN = dot(direction, N);

	if (!inside) {
		eta = 1.0 / eta;
	}
	   	
	float k = 1 - eta * eta * (1 - dotIN * dotIN);

	if (inside) {
		if (k < 0) {
			return false;
		}
		else {
			direction = normalize(eta * (direction - N * dotIN) - N * sqrt(k));
			return true;
		}
	}
	else {
		//assert(k > 0);
		direction = normalize(eta * (direction - N * dotIN) - N * sqrt(k));
		return true;
	}

}