#include "ray.h"


glm::vec3 Ray::reflect(glm::vec3 hitPos, glm::vec3 N, glm::vec3 V) {
	origin = hitPos;
	return normalize(2 * dot(N, V) * N - V);
}


glm::vec3 Ray::refract(glm::vec3 hitPos, glm::vec3 N, float eta) {
	origin = hitPos;
	float dotIN = dot(direction, N);
	float k = 1 - eta * eta * (1 - dotIN * dotIN);

	if (isInside) {
		if (k < 0) {
			return glm::vec3(0, 0, 0);
		}
		else {
			return normalize(eta * (direction - N * dotIN) - N * sqrt(k));
		}
	}
	else {
		assert(k > 0);
		return normalize(eta * (direction - N * dotIN) - N * sqrt(k));
	}

}