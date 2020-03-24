// The JSON library allows you to reference JSON arrays like C++ vectors and JSON objects like C++ maps.

#include "raytracer.h"

#include <iostream>
#include <fstream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "json.hpp"

using json = nlohmann::json;

const char *PATH = "scenes/";

double fov = 60;
colour3 background_colour(0, 0, 0);

json scene;

/****************************************************************************/

// here are some potentially useful utility functions

json find(json &j, const std::string key, const std::string value) {
	json::iterator it;
	for (it = j.begin(); it != j.end(); ++it) {
		if (it->find(key) != it->end()) {
			if ((*it)[key] == value) {
				return *it;
			}
		}
	}
	return json();
}

glm::vec3 vector_to_vec3(const std::vector<float> &v) {
	return glm::vec3(v[0], v[1], v[2]);
}

/****************************************************************************/

void choose_scene(char const *fn) {
	if (fn == NULL) {
		std::cout << "Using default input file " << PATH << "c.json\n";
		fn = "c";
	}

	std::cout << "Loading scene " << fn << std::endl;
	
	std::string fname = PATH + std::string(fn) + ".json";
	std::fstream in(fname);
	if (!in.is_open()) {
		std::cout << "Unable to open scene file " << fname << std::endl;
		exit(EXIT_FAILURE);
	}
	
	in >> scene;
	
	json camera = scene["camera"];
	// these are optional parameters (otherwise they default to the values initialized earlier)
	if (camera.find("field") != camera.end()) {
		fov = camera["field"];
		std::cout << "Setting fov to " << fov << " degrees.\n";
	}
	if (camera.find("background") != camera.end()) {
		background_colour = vector_to_vec3(camera["background"]);
		std::cout << "Setting background colour to " << glm::to_string(background_colour) << std::endl;
	}
}


/****************************************************************************/
bool hitSphere(glm::vec3 d, glm::vec3 e, glm::vec3 c, float r, glm::vec3& hit) {

	// solving quadratic equation
	double disc = pow(glm::dot(d, e - c), 2) - (glm::dot(d, d) * glm::dot(e - c, e - c) - r*r);

	if (disc < 0) {
		return false;
	}

	double sqrtDisc = sqrt(disc);
	double rest = glm::dot(-d, e - c) / glm::dot(d, d);
	
	double t;

	if (disc > 0) {
		double t1 = rest + sqrtDisc / glm::dot(d, d);
		double t2 = rest - sqrtDisc / glm::dot(d, d);
		if (t1 < t2) {
			t = t1;
		}
		else {
			t = t2;
		}
	}
	else {  // disc < 0 
		t = rest;
	}

	hit = e + float(t) * d;
	return true;
}

bool trace(const point3 &e, const point3 &s, colour3 &colour, bool pick) {
	// NOTE 1: This is a demo, not ray tracing code! You will need to replace all of this with your own code...
	// NOTE 2: You can work with JSON objects directly (like this sample code), read the JSON objects into your own data structures once and render from those (probably in choose_scene), or hard-code the objects in your own data structures and choose them by name in choose_scene; e.g. choose_scene('e') would pick the same scene as the one in "e.json". Your choice.
	// If you want to use this JSON library, https://github.com/nlohmann/json for more information. The code below gives examples of everything you should need: getting named values, iterating over arrays, and converting types.
	
	glm::vec3 light(0.5, 10, 2); // this should normally be taken from a file
	//json& lights = scene["lights"];


	// traverse the objects
	json &objects = scene["objects"];
	for (json::iterator it = objects.begin(); it != objects.end(); ++it) {
		json &object = *it;
		
		// every object in the scene will have a "type"
		if (object["type"] == "sphere") {
			// This is NOT ray-sphere intersection
			// Every sphere will have a position and a radius
			std::vector<float> pos = object["position"];
			point3 p = -(s - e) * pos[2];

			glm::vec3 c = vector_to_vec3(pos);
			glm::vec3 d = s - e;
			float r = float(object["radius"]);

			glm::vec3 hitPos;
			bool isHit = hitSphere(d, e, c, r, hitPos);

			if (isHit) {
				// Every object will have a material
				json &material = object["material"];
				std::vector<float> diffuse = material["diffuse"];
				colour = vector_to_vec3(diffuse);

				glm::vec3 L = glm::normalize(light - hitPos);
				glm::vec3 N = glm::normalize(hitPos - c);

				colour = colour * glm::dot(N,L);

				// This is NOT correct: it finds the first hit, not the closest
				return true;
			}
		}
	}

	return false;
}