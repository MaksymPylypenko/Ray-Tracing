// The JSON library allows you to reference JSON arrays like C++ vectors and JSON objects like C++ maps.

#include "raytracer.h"

#include <iostream>
#include <fstream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "json.hpp"

#include <cstdlib>

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
	// e - eye
	// d - pixel on the screen
	// c - center of a sphere
	// r - radius of a sphere

	// solving quadratic equation
	float disc = pow(dot(d, e - c), 2) - (dot(d, d) * dot(e - c, e - c) - r*r);

	if (disc < 0) {
		return false;
	}

	float sqrtDisc = sqrt(disc);
	float rest = dot(-d, e - c) / dot(d, d);
	
	float t;

	if (disc > 0) {
		float t1 = rest + sqrtDisc / dot(d, d);
		float t2 = rest - sqrtDisc / dot(d, d);
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

	hit = e + t * d;
	return true;
}

/****************************************************************************/

bool hitPlane(glm::vec3 d, glm::vec3 e, glm::vec3 a, glm::vec3 n, glm::vec3& hit) {
	// e - eye
	// d - pixel on the screen
	// a - position of a plane
	// n - normal vector    

	float dotND = dot(n, d); // sign tells us which side of a plane was hit

	if (abs(dotND) > 0.001) {
		float t = dot(n, a - e) / abs(dotND);
		if (t > 0) { //we have a collision
			hit = e + t * d;
			return true;
		}
	}	

	return false;
}


/****************************************************************************/

bool trace(const point3 &e, const point3 &s, colour3 &colour, bool pick) {

	// NOTE 1: This is a demo, not ray tracing code! You will need to replace all of this with your own code...
	// NOTE 2: You can work with JSON objects directly (like this sample code), read the JSON objects into your own data structures once and render from those (probably in choose_scene), or hard-code the objects in your own data structures and choose them by name in choose_scene; e.g. choose_scene('e') would pick the same scene as the one in "e.json". Your choice.
	// If you want to use this JSON library, https://github.com/nlohmann/json for more information. The code below gives examples of everything you should need: getting named values, iterating over arrays, and converting types.
	
	//json& lights = scene["lights"]; // TODO use lights from a file
	glm::vec3 light(-5, 10, 5);

	// traverse the objects
	json &objects = scene["objects"];
	for (json::iterator it = objects.begin(); it != objects.end(); ++it) {
		json &object = *it;
		
		// every object in the scene will have a "type"
		if (object["type"] == "sphere") {
			// Every sphere will have a position and a radius
			std::vector<float> pos = object["position"];

			glm::vec3 c = vector_to_vec3(pos);				
			glm::vec3 d = s - e; // from viewer
			float r = float(object["radius"]);

			glm::vec3 hitPos;								
			bool isHit = hitSphere(d, e, c, r, hitPos);

			if (isHit) {
				// Every object will have a material
				json &material = object["material"];
				glm::vec3 ambient = vector_to_vec3(material["ambient"]);
				glm::vec3 diffuse = vector_to_vec3(material["diffuse"]);
				//glm::vec3 specular = vector_to_vec3(material["specular"]);
				float shininess = material["shininess"];
						
				glm::vec3 L = glm::normalize(light - hitPos); // to light 
				glm::vec3 N = glm::normalize(hitPos - c); // normal
				glm::vec3 V = glm::normalize(e-s); // to viewer

				float dotLN = glm::dot(L,N);	
				diffuse *= dotLN;

				if (dotLN < 0) { // cant see an object from here					
					diffuse = glm::vec3(0.0, 0.0, 0.0);
					//specular = glm::vec3(0.0, 0.0, 0.0);
				}
				else {
					glm::vec3 R = glm::normalize(2 * dotLN * N - L); // perfect light reflection
					float dotRV = glm::dot(R, V);
					if (dotRV < 0) {
						//specular = glm::vec3(0.0, 0.0, 0.0);
					}
					else {
						//specular *= glm::pow(dotRV, shininess);
					}											
				}							

				colour = ambient + diffuse; //+specular;

				// This is NOT correct: it finds the first hit, not the closest
				return true;
			}
		}


		if (object["type"] == "plane") {	
			glm::vec3 a = vector_to_vec3(object["position"]);
			glm::vec3 d = s - e; // from viewer
			glm::vec3 N = vector_to_vec3(object["normal"]);
			glm::vec3 hitPos;

			bool isHit = hitPlane(d, e, a, N, hitPos);

			if (isHit) {
				// Every object will have a material
				json& material = object["material"];
				glm::vec3 ambient = vector_to_vec3(material["ambient"]);
				glm::vec3 diffuse = vector_to_vec3(material["diffuse"]);
				glm::vec3 specular = vector_to_vec3(material["specular"]);
				float shininess = material["shininess"];

				glm::vec3 L = glm::normalize(light - hitPos); // to light 
				glm::vec3 V = glm::normalize(e - s); // to viewer

				float dotLN = glm::dot(L, N);
				diffuse *= dotLN;

				if (dotLN < 0) { // cant see an object from here					
					diffuse = glm::vec3(0.0, 0.0, 0.0);
					specular = glm::vec3(0.0, 0.0, 0.0);
				}
				else {
					glm::vec3 R = glm::normalize(2 * dotLN * N - L); // perfect light reflection
					float dotRV = glm::dot(R, V);
					if (dotRV < 0) {
						specular = glm::vec3(0.0, 0.0, 0.0);
					}
					else {
						specular *= glm::pow(dotRV, shininess);
					}
				}

				colour = ambient + diffuse + specular;

				// This is NOT correct: it finds the first hit, not the closest
				return true;
			}
		}
	}

	return false;
}