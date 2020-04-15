#ifndef ray_h // include guard
#define ray_h

#include "utility/scene_adapter.h"

// Initial ray configuration
const float MIN_RAY_LEN = 0.000001f;
const float MAX_RAY_LEN = 999.0f;
const int REFLECTIVE_BOUNCES = 5;
const bool IS_INSIDE = false;

// returns [fov]
float loadScene(char* fn);

// This function is called recursively, returns a final [colour].
glm::vec3 trace(glm::vec3 rayOrigin, glm::vec3 rayDir, int bouncesLeft, 
	bool isInside, bool showDebug);

// Similar to trace, except it finds the first hit, not the closest one
bool isShadow(glm::vec3 rayOrigin, glm::vec3 rayDir, float maxRayLen = 999);

#endif ray_h