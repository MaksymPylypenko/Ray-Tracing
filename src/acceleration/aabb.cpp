#include "acceleration.h"


bool isHitBounds(Ray ray, glm::vec3 min, glm::vec3 max) {
	/// This is a variation of a classic AABB intersection test
	/// I used optimization advices described in this article: https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525
	
	glm::vec3 invRayDir = 1.0f / ray.direction;
	glm::vec3 t0 = (min - ray.origin) * invRayDir;
	glm::vec3 t1 = (max - ray.origin) * invRayDir;

	glm::vec3 tmin = glm::min(t0, t1);
	glm::vec3 tmax = glm::max(t0, t1);

	float lowest = glm::max(ray.minLen, glm::max(tmin[0], glm::max(tmin[1], tmin[2])));
	float highest = glm::min(ray.maxLen, glm::min(tmax[0], glm::min(tmax[1], tmax[2])));

	return lowest <= highest;
}