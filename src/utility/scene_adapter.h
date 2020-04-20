#ifndef scene_adapter_h // include guard
#define scene_adapter_h

#include <iostream>					// cout
#include <fstream>					// in
#include <glm/gtx/string_cast.hpp>	// to_string

#include "json.hpp"

#include "../models/model.h"
#include "../acceleration/acceleration.h"

#include "../light/light.h"

using json = nlohmann::json;

class SceneAdapter {
public:	
	json scene;
	double fov = 60;
	bool antialiasing = false;
	glm::vec3 background_colour;
	std::vector<Object*> objects;
	std::vector<Light*> lights;

	void chooseScene(char const* fn);
	void loadThings();
};

#endif scene_adapter_h