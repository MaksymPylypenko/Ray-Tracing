#ifndef light_h // include guard
#define light_h

#include "common.h"
#include "light.h"
#include "model.h"

#include <glm/glm.hpp>  // glm
#include <vector>		// std::vector

using namespace std;

const float MAX_RAY_LEN = 999.0f;
const float MIN_RAY_LEN = 0.000001f;


class Light {
public:
	virtual glm::vec3 apply(std::vector<Object*> objects, Material * material,
		glm::vec3 N, glm::vec3 V, glm::vec3 hitPos);
};

class Ambient : public Light {
public:
	glm::vec3 colour;
	glm::vec3 apply(std::vector<Object*> objects, Material* material,
		glm::vec3 N, glm::vec3 V, glm::vec3 hitPos);
};

class Point : public Light {
public:
	glm::vec3 colour;
	glm::vec3 position;
	glm::vec3 apply(std::vector<Object*> objects, Material* material,
		glm::vec3 N, glm::vec3 V, glm::vec3 hitPos);
};

class Directional : public Light {
public:
	glm::vec3 colour;
	glm::vec3 direction;
	glm::vec3 apply(std::vector<Object*> objects, Material* material,
		glm::vec3 N, glm::vec3 V, glm::vec3 hitPos);
};


class Spot : public Light {
public:
	glm::vec3 colour;
	glm::vec3 position;
	glm::vec3 direction;
	float cutoff;

	glm::vec3 apply(std::vector<Object*> objects, Material* material,
		glm::vec3 N, glm::vec3 V, glm::vec3 hitPos);
};

#endif light_h