#ifndef model_h // include guard
#define model_h

#include "../common.h"
#include "model.h"

#include <glm/glm.hpp>  // glm
#include <vector>		// std::vector
#include <algorithm>	// std::max

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


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
	glm::vec3 center;	
	glm::vec3 normal;
	
	Material* material;
	float rayLen;

	virtual bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, 
		float minRayLen, float maxRayLen, bool inside = false);

	// for texture
	bool texture = false;
	virtual void checkersTexture(glm::vec3 hitPos);
	   
	virtual void debug();
};


class Sphere : public Object {
public:
	float radius;
	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, 
		float minRayLen, float maxRayLen, bool inside = false) override;

	void checkersTexture(glm::vec3 hitPos) override;

	float scale = 12.0;
	void debug() override;
};


class Plane : public Object {
public:
	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, 
		float minRayLen, float maxRayLen, bool inside = false) override;

	// for textures
	glm::vec3 axisU;
	glm::vec3 axisV;
	float scale = 1.68;
	void alignTextureAxes();
	void checkersTexture(glm::vec3 hitPos);

	void debug() override;
};


class Triangle : public Object {
public:
	std::vector<glm::vec3> points;	

	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, 
		float minRayLen, float maxRayLen, bool inside = false) override;
	   
	void debug() override;

	// for acceleration
	int nodeID;
	void setBarycenter();
};



class Mesh : public Object {
/// Holds as a set of [Triangles]
///
/// To hit a [Mesh] the ray should hit a triangle.
/// Note, only the closest hit is considered
public:
	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir,
		float minRayLen, float maxRayLen, bool inside = false) override;

	// acceleration
	glm::vec3 min;
	glm::vec3 max;
	std::vector<Triangle*> triangles;
	void findBounds();

	// transformations
	glm::quat q;
	void translate(glm::vec3 vector);
	void scale(float scale);
	void addQuaternion(glm::vec3 axis, float angle);
	void rotate();

	void debug() override;
};


#endif model_h