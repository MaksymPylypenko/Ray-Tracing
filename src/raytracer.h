#ifndef raytracer_h // include guard
#define raytracer_h

#include "utility/scene_adapter.h"
#include "ray.h"

// loads the scene
void loadScene(char* fn, float& fov, bool& antialiasing);

// This function is called recursively, returns a final [colour].
glm::vec3 trace(Ray ray);

// Similar to trace, except it finds the first hit, not the closest one
bool traceShadow(Ray ray);


#endif raytracer_h