#ifndef light_h // include guard
#define light_h

#include "../common.h"
#include "../models/model.h"
#include "light.h"

#include <glm/glm.hpp>  // glm
#include <vector>		// std::vector


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


class Area : public Light {
public:
	glm::vec3 colour;
	glm::vec3 position;
	glm::vec3 dirU;
	glm::vec3 dirV;
	glm::vec3 normal;
	float distU;
	float distV;

	glm::vec3 apply(std::vector<Object*> objects, Material* material,
		glm::vec3 N, glm::vec3 V, glm::vec3 hitPos);
};


#endif light_h