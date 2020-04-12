#ifndef model_h // include guard
#define model_h

#include "common.h"
#include "model.h"

#include <glm/glm.hpp>  // glm
#include <vector>		// std::vector

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/*************************************************************************/
/* Objects */

class Material {
public:
	glm::vec3 Ka;
	glm::vec3 Kd;
	glm::vec3 Ks;
	float shininess;
	glm::vec3 transmission;
	glm::vec3 reflection;
	float refraction;
};


class Object {
public:	
	Material * material;
	glm::vec3 normal;
	float rayLen;

	virtual bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, 
		float minRayLen, float maxRayLen, bool inside = false);

	virtual void debug();
};

class Sphere : public Object {
public:
	glm::vec3 center;
	float radius;
	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, 
		float minRayLen, float maxRayLen, bool inside = false) override;

	void debug() override;
};

class Plane : public Object {
public:
	glm::vec3 position;
	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, 
		float minRayLen, float maxRayLen, bool inside = false) override;

	void debug() override;
};



class Triangle : public Object {
public:
	std::vector<glm::vec3> points;
	int nodeID; // used for the Octree construction
	glm::vec3 Triangle::getBarycenter();

	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, 
		float minRayLen, float maxRayLen, bool inside = false) override;

	void debug() override;
};



class Mesh : public Object {
public:
	glm::vec3 min;
	glm::vec3 max;
	std::vector<Triangle*> triangles;

	void findSlabs();

	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir,
		float minRayLen, float maxRayLen, bool inside = false) override;

	void translate(glm::vec3 vector);
	void scale(float scale, bool findOrigin = true);
	void rotate(glm::vec3 axis, float angle, bool findOrigin = false);

	void debug() override;
};

/*************************************************************************/
/* Acceleration */


class BoundingVolume : public Object {
public:

	BoundingVolume* children[8]; // Using Octree to insert BVH nodes by proximity
	Mesh* mesh;

	bool isLeave = false;

	bool build(Mesh* mesh, int threshold = 4, int maxDepth = 10, int currDepth = 0);
	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside);
	void debug();
};


#endif model_h