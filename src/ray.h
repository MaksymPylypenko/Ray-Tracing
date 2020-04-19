#ifndef ray_h // include guard
#define ray_h

#include <glm/glm.hpp>  // glm

class Ray {
public:
	glm::vec3 origin;
	glm::vec3 direction;

	float minLen = 0.000001f;
	float maxLen = 999.0f;

	int bouncesLeft = 5;
	bool isInside = false;

	glm::vec3 reflect(glm::vec3 hitPos, glm::vec3 N, glm::vec3 V);
	glm::vec3 refract(glm::vec3 hitPos, glm::vec3 N, float eta);
};

#endif ray_h