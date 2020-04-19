#ifndef ray_h // include guard
#define ray_h

#include <glm/glm.hpp>  // glm

class Ray {
public:
	glm::vec3 origin;
	glm::vec3 direction;

	float minLen = 0.0001f;
	float maxLen = 999.0f;

	int bouncesLeft = 5;

	void reflect(glm::vec3 hitPos, glm::vec3 N, glm::vec3 V);
	bool refract(glm::vec3 hitPos, glm::vec3 N, float eta, bool inside);

	bool blendingMode = false;
	bool negativeOn = false;
};

#endif ray_h