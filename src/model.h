#ifndef model_h // include guard
#define model_h

#include "common.h"
#include "model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>      // std::ifstream
#include <vector>		// std::vector
#include <sstream>      // std::istringstream
#include <string>       // std::string

using namespace std;

class Material {
public:
	glm::vec3 Ka;
	glm::vec3 Kd;
	glm::vec3 Ks;
	float shininess;
	glm::vec3 transmission;
	glm::vec3 reflection;
	float medium;	   		
};


class Object {
public:	
	Material * material;
	glm::vec3 normal;
	float rayLen;

	virtual bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen);
};

class Sphere : public Object {
public:
	glm::vec3 center;
	float radius;
	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen) override;
};

class Plane : public Object {
public:
	glm::vec3 position;
	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen) override;
};


class Triangle : public Object {
public:
	glm::vec3 a;
	glm::vec3 b;
	glm::vec3 c;
	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen) override;
};



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

#endif model_h