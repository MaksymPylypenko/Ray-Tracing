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
	MeshHierarchy* children[8] = { NULL }; // Using Octree to insert BVH nodes by proximity
	Mesh* mesh = NULL;

	bool isLeave = false;

	bool build(Mesh* mesh, int threshold = 4, int maxDepth = 20, int currDepth = 0);
	bool isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) override;
	void debug();
};


#endif accel_h