#ifndef accel_h // include guard
#define accel_h

#include "model.h"
#include "acceleration.h"


/// AABB intersection test
/// 
/// Allows to check if the [Ray] intersected a bounding volume
/// Used optimization advices described in this article: https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525
bool isHitBounds(glm::vec3 min, glm::vec3 max, glm::vec3 rayOrigin, glm::vec3 rayDir,
	float minRayLen, float maxRayLen);


class Mesh : public Object {
public:
	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir,
		float minRayLen, float maxRayLen, bool inside = false) override;

	// acceleration
	Triangle* closest;
	std::vector<Triangle*> triangles;
	void findBounds() override;


	// transformations
	glm::quat q;
	void translate(glm::vec3 vector);
	void scale(float scale);
	void addQuaternion(glm::vec3 axis, float angle);
	void rotate();

	void debug() override;
};


class MeshHierarchy : public Object {
public:

	MeshHierarchy* children[8]; // Using Octree to insert BVH nodes by proximity
	Mesh* mesh;

	bool isLeave = false;

	bool build(Mesh* mesh, int threshold = 4, int maxDepth = 20, int currDepth = 0);
	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) override;
	void debug();

	void findBounds() override;
};


/// Just to make things easier, I will have another hierarchy 
/// for high level objects like: [Sphere], [Plane], [MeshHierarchy]

class BoundingVolume : public Object {
public:
	Object* closest;
	std::vector<Object*> objects;
	void findBounds() override;

	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir,
		float minRayLen, float maxRayLen, bool inside = false) override;
};


class BVH {
public:

	BVH* children[8]; // Using Octree to insert BVH nodes by proximity
	BoundingVolume* bv;
	Object* closest;

	bool isLeave = false;

	bool build(BoundingVolume* bv, int threshold = 1);
	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside);
};


#endif accel_h