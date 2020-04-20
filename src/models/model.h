#ifndef model_h // include guard
#define model_h

#include "../common.h"
#include "../utility/texture.h"
#include "../ray.h"


#include <glm/glm.hpp>  // glm
#include <vector>		// std::vector
#include <algorithm>	// std::max
#include <math.h>       // fmod 

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
	
	Material * material = nullptr;
	Texture* texture = nullptr;
	bool isNegative;

	// Hit information
	float rayLen = 0.0;
	bool inside = false;

	virtual bool isHit(Ray ray);	
	virtual void applyTexture(glm::vec3 hitPos);	   
	virtual void debug();
};


class Sphere : public Object {
public:
	float radius;
	bool isHit(Ray ray) override;

	void applyTexture(glm::vec3 hitPos) override;
	void debug() override;
};


class Plane : public Object {
public:
	bool isHit(Ray ray) override;

	// for textures
	glm::vec3 axisU;
	glm::vec3 axisV;
	void alignTextureAxes();
	void applyTexture(glm::vec3 hitPos);

	void debug() override;
};


class Triangle : public Object {
public:
	std::vector<glm::vec3> points;	

	/// Object's [normal] will change depending on the nature of the hit.
	/// For instance, it will be different for outside & inside hits. 
	/// A [fixedNormal] is generated when triangle is created and is based 
	/// purely on the winding.
	glm::vec3 fixedNormal;
	void setNormal();
	bool hitPlane(Ray ray);

	bool isHit(Ray ray) override;	   
	void debug() override;

	void applyTexture(glm::vec3 hitPos);

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
	bool isHit(Ray ray) override;

	std::vector<Triangle*> triangles;

	// acceleration
	glm::vec3 min;
	glm::vec3 max;
	void resetOrigin();


	// transformations
	void resetNormals();
	void resetBarycenters();
	glm::quat q;
	void translate(glm::vec3 vector);
	void scale(float scale);
	void addQuaternion(glm::vec3 axis, float angle);
	void rotate();

	void debug() override;
};


#endif model_h