#include "ray.h"


void Ray::reflect(glm::vec3 hitPos, glm::vec3 N, glm::vec3 V) {
	origin = hitPos;
	direction = normalize(2 * dot(N, V) * N - V);
}


void Ray::refract(glm::vec3 hitPos, glm::vec3 N, float eta) {
	origin = hitPos;
	float dotIN = dot(direction, N);
	float k = 1 - eta * eta * (1 - dotIN * dotIN);

	if (isInside) {
		if (k < 0) {
			direction = glm::vec3(0, 0, 0);
			return;
		}
		else {
			direction = normalize(eta * (direction - N * dotIN) - N * sqrt(k));
			return;
		}
	}
	else {
		assert(k > 0);
		direction = normalize(eta * (direction - N * dotIN) - N * sqrt(k));
		return;
	}

}