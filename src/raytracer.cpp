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

enum ObjectType { Sphere, Plane, Mesh };

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

glm::vec3 getAmbient(json& object) {
	json& material = object["material"];
	try {
		return vector_to_vec3(material["ambient"]);
	}
	catch (...) {
		return glm::vec3(0.0, 0.0, 0.0);
	}
}

glm::vec3 getDiffuse(json& object) {
	json& material = object["material"];
	try {
		return vector_to_vec3(material["diffuse"]);
	}
	catch (...) {
		return glm::vec3(0.0, 0.0, 0.0);
	}
}

glm::vec3 getSpecular(json& object) {
	json& material = object["material"];
	try {
		return vector_to_vec3(material["specular"]);
	}
	catch (...) {
		return glm::vec3(0.0, 0.0, 0.0);
	}
}


float getShininess(json& object) {
	json& material = object["material"];
	try {
		return (material["shininess"]);
	}
	catch (...) {
		return 0.0;
	}
}


/****************************************************************************/

bool hitSphere(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 sphereCenter, float radius, glm::vec3& hit) {

	float bSq = pow(dot(rayDir, rayOrigin - sphereCenter), 2);
	float FourAC = dot(rayDir, rayDir) * dot(rayOrigin - sphereCenter, rayOrigin - sphereCenter) - radius * radius;
	float discriminant = bSq - FourAC;

	if (discriminant < 0) {
		return false;
	}

	float sqrtDisc = sqrt(discriminant);
	float rest = dot(-rayDir, rayOrigin - sphereCenter) / dot(rayDir, rayDir);
	
	float t;

	if (discriminant > 0) {
		float t1 = rest + sqrtDisc / dot(rayDir, rayDir);
		float t2 = rest - sqrtDisc / dot(rayDir, rayDir);
		if (t1 < t2) {
			t = t1;
		}
		else {
			t = t2;
		}
	}
	else {  // discriminant < 0 
		t = rest;
	}

	hit = rayOrigin + t * rayDir;
	return true;
}

/****************************************************************************/

bool hitPlane(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 planePos, glm::vec3 N, glm::vec3& hit) {
	float dotND = dot(N, rayDir); // sign tells us which side of a plane was hit
	if (dotND < 0) {
		float t = dot(N, planePos - rayOrigin) / dotND;
		hit = rayOrigin + t * rayDir;
		return true;		
	}	
	return false;
}


bool hitBoundingBox(glm::vec3 rayOrigin, glm::vec3 rayDir, 
	std::vector<std::vector<std::vector<float>>> mesh, glm::vec3& hit) {

	float minX = mesh[0][0][0]; // first triangle, first point, x
	float minY = mesh[0][0][1];
	float minZ = mesh[0][0][2];
	float maxX = mesh[0][0][0];
	float maxY = mesh[0][0][1];
	float maxZ = mesh[0][0][2];

	for (std::vector<std::vector<float>> triangle :mesh) {
		for (std::vector<float> point : triangle) {
			glm::vec3 curr = vector_to_vec3(point);
			minX = (curr.x < minX ? curr.x : minX);
			minY = (curr.y < minY ? curr.y : minY);
			minZ = (curr.z < minZ ? curr.z : minZ);
			maxX = (curr.x > maxX ? curr.x : maxX);
			maxY = (curr.y > maxY ? curr.y : maxY);
			maxZ = (curr.z > maxZ ? curr.z : maxZ);			
		}
	}

	// @TODO return true if inside the bounding box

	return false;
}


bool hitTriangle(glm::vec3 rayOrigin, glm::vec3 rayDir,
	std::vector<std::vector<float>> triangle, glm::vec3& hitPos, glm::vec3& N) {

	glm::vec3 a = vector_to_vec3(triangle[0]);
	glm::vec3 b = vector_to_vec3(triangle[1]);
	glm::vec3 c = vector_to_vec3(triangle[2]);

	N = normalize(cross((b - a),(c - a)));
	bool isPlaneHit = hitPlane(rayOrigin, rayDir, a, N, hitPos);
	
	if (isPlaneHit) {		
		float axProj = dot(cross((b - a), (hitPos - a)), N);
		float bxProj = dot(cross((c - b), (hitPos - b)), N);
		float cxProj = dot(cross((a - c), (hitPos - c)), N);

		if (axProj>0 && bxProj>0 && cxProj>0) { 
			return true;
		}	
	}	
	return false;
}


/****************************************************************************/

glm::vec3 phong( glm::vec3 L, glm::vec3 N, glm::vec3 V,
	glm::vec3 ambient = glm::vec3(0.0, 0.0, 0.0), 
	glm::vec3 diffuse = glm::vec3(0.0, 0.0, 0.0), 
	glm::vec3 specular = glm::vec3(0.0, 0.0, 0.0), 
	float shininess = 0.0) {
	
	float dotLN = glm::dot(L, N);
	if (dotLN < 0) { // light does not reach the object					
		diffuse = glm::vec3(0.0, 0.0, 0.0);
		specular = glm::vec3(0.0, 0.0, 0.0);
	}
	else {
		diffuse *= dotLN;
		if (specular != glm::vec3(0.0, 0.0, 0.0)) { // only calculate this for non-zero specular component
			glm::vec3 R = glm::normalize(2 * dotLN * N - L); 
			float dotRV = glm::dot(R, V);
			if (dotRV < 0) { // viewer doesn't see the bright spot
				specular = glm::vec3(0.0, 0.0, 0.0);
			}
			else {
				specular *= glm::pow(dotRV, shininess);
			}
		}		
	}
	return ambient + diffuse + specular;
}


/****************************************************************************/

bool trace(const point3 &rayOrigin, const point3 &pixel, colour3 &colour, bool pick) {

	// NOTE 1: This is a demo, not ray tracing code! You will need to replace all of this with your own code...
	// NOTE 2: You can work with JSON objects directly (like this sample code), read the JSON objects into your own data structures once and render from those (probably in choose_scene), or hard-code the objects in your own data structures and choose them by name in choose_scene; e.g. choose_scene('e') would pick the same scene as the one in "e.json". Your choice.
	// If you want to use this JSON library, https://github.com/nlohmann/json for more information. The code below gives examples of everything you should need: getting named values, iterating over arrays, and converting types.
	
	//json& lights = scene["lights"]; // TODO use lights from a file
	glm::vec3 light(5, 10, 2);

	bool didHit = false;
	glm::vec3 firstHit;
	float firstRayLen = 999;

	// traverse the objects
	json &objects = scene["objects"];
	for (json::iterator it = objects.begin(); it != objects.end(); ++it) {
		json &object = *it;
		

		if (object["type"] == "plane") {
			glm::vec3 a = vector_to_vec3(object["position"]);
			glm::vec3 rayDir = pixel - rayOrigin;
			glm::vec3 N = vector_to_vec3(object["normal"]);
			glm::vec3 hitPos;

			bool isHit = hitPlane(rayOrigin, rayDir, a, N, hitPos);

			if (isHit) {
				float rayLen = glm::length(hitPos-rayOrigin);
				if (rayLen < firstRayLen) {
					glm::vec3 ambient = getAmbient(object);
					glm::vec3 diffuse = getDiffuse(object);
					//glm::vec3 specular = getSpecular(object); // planes don't have this component in the examples ...
					//float shininess = getShininess(object);
					glm::vec3 L = glm::normalize(light - hitPos);
					glm::vec3 V = glm::normalize(pixel - hitPos);
					colour = phong(L, N, V, ambient, diffuse); //specular, shininess);
					firstHit = hitPos; // assuming this is the longest ray possible	
					firstRayLen = rayLen;
					
				}
				didHit = true;
			}
		}

		if (object["type"] == "mesh") {
	
			for (std::vector<std::vector<float>> triangle : object["triangles"]) {
				glm::vec3 rayDir = pixel - rayOrigin;
				glm::vec3 hitPos;
				glm::vec3 N;
				bool isHit = hitTriangle(rayOrigin, rayDir, triangle, hitPos, N);
				if (isHit) {
					float rayLen = glm::length(hitPos-rayOrigin);
					if (rayLen < firstRayLen) {
						glm::vec3 ambient = getAmbient(object);
						glm::vec3 diffuse = getDiffuse(object);
						glm::vec3 L = normalize(light - hitPos);
						glm::vec3 V = normalize(pixel - rayOrigin);
						colour = phong(L, N, V, ambient, diffuse);
						firstHit = hitPos;
						firstRayLen = rayLen;
						
					}
					didHit = true;
				}
			}
		}

		// every object in the scene will have a "type"
		if (object["type"] == "sphere") {
			// Every sphere will have a position and a radius
			std::vector<float> pos = object["position"];

			glm::vec3 c = vector_to_vec3(pos);				
			glm::vec3 rayDir = pixel - rayOrigin; // from viewer
			float r = float(object["radius"]);

			glm::vec3 hitPos;								
			bool isHit = hitSphere(rayOrigin, rayDir, c, r, hitPos);

			if (isHit) {
				float rayLen = glm::length(hitPos-rayOrigin);
				if (rayLen < firstRayLen) {
					glm::vec3 ambient = getAmbient(object);
					glm::vec3 diffuse = getDiffuse(object);
					glm::vec3 specular = getSpecular(object);
					float shininess = getShininess(object);
					glm::vec3 L = normalize(light - hitPos);
					glm::vec3 N = normalize(hitPos - c);
					glm::vec3 V = normalize(pixel - hitPos);
					colour = phong(L, N, V, ambient, diffuse, specular, shininess);
					firstHit = hitPos;
					firstRayLen = rayLen;					
				}
				didHit = true;
			}
		}		
	}

	if (didHit) {
		return true;
	}

	return false;
}