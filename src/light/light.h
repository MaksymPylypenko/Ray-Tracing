#ifndef light_h // include guard
#define light_h

#include "../common.h"
#include "../models/model.h"
#include "light.h"

#include <glm/glm.hpp>  // glm
#include <vector>		// std::vector


class Light {
public:
	virtual glm::vec3 apply(glm::vec3 hitPos, glm::vec3 V, glm::vec3 N, Material* material);
};


class Ambient : public Light {
public:
	glm::vec3 colour;
	glm::vec3 apply(glm::vec3 hitPos, glm::vec3 V, glm::vec3 N, Material * material);
};


class Point : public Light {
public:
	glm::vec3 colour;
	glm::vec3 position;
	glm::vec3 apply(glm::vec3 hitPos, glm::vec3 V, glm::vec3 N, Material* material);
};


class Directional : public Light {
public:
	glm::vec3 colour;
	glm::vec3 direction;
	glm::vec3 apply(glm::vec3 hitPos, glm::vec3 V, glm::vec3 N, Material* material);
};


class Spot : public Light {
public:
	glm::vec3 colour;
	glm::vec3 position;
	glm::vec3 direction;
	float cutoff = 0.0f;

	glm::vec3 apply(glm::vec3 hitPos, glm::vec3 V, glm::vec3 N, Material* material);
};


class Area : public Light {
public:
	glm::vec3 colour;
	glm::vec3 position;
	glm::vec3 dirU;
	glm::vec3 dirV;
	glm::vec3 normal;
	float distU = 0.0f;
	float distV = 0.0f;

	glm::vec3 apply(glm::vec3 hitPos, glm::vec3 V, glm::vec3 N, Material* material);
	Object* makeLamp();
};


#endif light_h