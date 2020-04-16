#ifndef accel_h // include guard
#define accel_h

#include "../models/model.h"
#include "acceleration.h"

/// Allows to check if the [Ray] intersected a bounding volume
bool isHitBounds(glm::vec3 min, glm::vec3 max, glm::vec3 rayOrigin, glm::vec3 rayDir,
	float minRayLen, float maxRayLen);


class MeshHierarchy : public Object {
/// Allows to speed up the rendering of a very complex object (e.g Teapot)
///
/// I use a spatial data structure similar to an "Octree" to separate triangles 
/// into smaller groups based on proximity. This allows to group non-empty nodes 
/// on different depths to create bounding volumes.
public:
	MeshHierarchy* children[8]; // Using Octree to insert BVH nodes by proximity
	Mesh* mesh;

	bool isLeave = false;

	bool build(Mesh* mesh, int threshold = 4, int maxDepth = 20, int currDepth = 0);
	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) override;
	void debug();

	void findBounds() override;
};


class BoundingVolume  {
/// Holds as a set of high level objects like: [Spheres], [Planes], [MeshHierarchies].
///
/// To hit a [BoundingVolume] the ray should hit an object.
/// Note, only the closest hit is considered
public:

	glm::vec3 center;
	glm::vec3 min;
	glm::vec3 max;

	Object* closest;
	std::vector<Object*> objects;
	void findBounds();

	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir,
		float minRayLen, float maxRayLen, bool inside = false);
};


class BVH {
///	Allows to speed up the rendering of a large set of objects (e.g. 10+)
///
/// This is very similar to a [MeshHierarchy]. I just treat high level objects as triangles.
public:

	BVH* children[8]; 
	BoundingVolume* bv;
	Object* closest;

	bool isLeave = false;

	bool build(BoundingVolume* bv, int threshold = 1);
	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside);
};


#endif accel_h