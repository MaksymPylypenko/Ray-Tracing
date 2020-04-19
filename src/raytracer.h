#ifndef raytracer_h // include guard
#define raytracer_h

#include "utility/scene_adapter.h"
#include "ray.h"

// returns [fov]
float loadScene(char* fn);

// This function is called recursively, returns a final [colour].
glm::vec3 trace(Ray ray, bool showDebug);

// Similar to trace, except it finds the first hit, not the closest one
bool isShadow(Ray ray);


#endif raytracer_h