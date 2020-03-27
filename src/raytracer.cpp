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

bool isHitSphere(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 sphereCenter, float radius, glm::vec3& nearestHit) {

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

	nearestHit = rayOrigin + t * rayDir;
	return true;
}

/****************************************************************************/

bool isHitPlane(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 planePos, glm::vec3 N, glm::vec3& nearestHit) {
	float dotND = dot(N, rayDir); // sign tells us which side of a plane was nearestHit
	if (dotND < 0) {
		float t = dot(N, planePos - rayOrigin) / dotND;
		nearestHit = rayOrigin + t * rayDir;
		return true;		
	}	
	return false;
}


bool nearestHitBoundingBox(glm::vec3 rayOrigin, glm::vec3 rayDir, 
	std::vector<std::vector<std::vector<float>>> mesh, glm::vec3& nearestHit) {

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


bool isHitTriangle(glm::vec3 rayOrigin, glm::vec3 rayDir,
	std::vector<std::vector<float>> triangle, glm::vec3& hitPos, glm::vec3& N) {

	glm::vec3 a = vector_to_vec3(triangle[0]);
	glm::vec3 b = vector_to_vec3(triangle[1]);
	glm::vec3 c = vector_to_vec3(triangle[2]);

	N = normalize(cross((b - a),(c - a)));
	bool isPlanenearestHit = isHitPlane(rayOrigin, rayDir, a, N, hitPos);
	
	if (isPlanenearestHit) {		
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
	glm::vec3 Kd = glm::vec3(0.0, 0.0, 0.0), // material diffuse
	glm::vec3 Ks = glm::vec3(0.0, 0.0, 0.0), // material specular
	float shininess = 0.0,	// material
	glm::vec3 Ids = glm::vec3(0.0, 0.0, 0.0)) // light diffuse or ligth specular ..oh well, ok
{
	
	float dotLN = glm::dot(L, N);
	if (dotLN < 0) { // light does not reach the object					
		Kd = glm::vec3(0.0, 0.0, 0.0);
		Ks = glm::vec3(0.0, 0.0, 0.0);
	}
	else {
		Kd *= dotLN;
		if (Ks != glm::vec3(0.0, 0.0, 0.0)) { // only calculate this for non-zero specular component
			glm::vec3 R = glm::normalize(2 * dotLN * N - L); 
			float dotRV = glm::dot(R, V);
			if (dotRV < 0) { // viewer doesn't see the bright spot
				Ks = glm::vec3(0.0, 0.0, 0.0);
			}
			else {
				Ks *= glm::pow(dotRV, shininess);
			}
		}		
	}
	return Kd*Ids + Ks*Ids;
}


glm::vec3 applyLights(json& object,json& lights, 
	glm::vec3 N, glm::vec3 pixel, glm::vec3 hitPos) {
	
	glm::vec3 V = normalize(pixel - hitPos);
	glm::vec3 colour;

	// Material
	json& material = object["material"];	

	glm::vec3 Ka, Kd, Ks;
	float shininess;

	if (material.find("ambient") != material.end()) {
		Ka = vector_to_vec3(material["ambient"]);
	}
	else {
		Ka = glm::vec3(0.0, 0.0, 0.0);
	}

	if (material.find("diffuse") != material.end()) {
		Kd = vector_to_vec3(material["diffuse"]);
	}
	else {
		Kd = glm::vec3(0.0, 0.0, 0.0);
	}

	if (material.find("specular") != material.end()) {
		Ks = vector_to_vec3(material["specular"]);
	}
	else {
		Ks = glm::vec3(0.0, 0.0, 0.0);
	}

	if (material.find("shininess") != material.end()) {
		shininess =  material["shininess"];
	}
	else {
		shininess = 0.0;
	}
	   
	// Lights 
	for (json::iterator it = lights.begin(); it != lights.end(); ++it) {
		json& light = *it;

		if (light["type"] == "ambient") {
			glm::vec3 Ia = vector_to_vec3(light["color"]);
			colour += Ka * Ia;
		}
		else if (light["type"] == "point") {
			glm::vec3 L = normalize(vector_to_vec3(light["position"]) - hitPos);
			colour += phong(L, N, V, Kd, Ks, shininess, vector_to_vec3(light["color"]));
		}
		else if (light["type"] == "directional") {
			glm::vec3 L = normalize(-vector_to_vec3(light["direction"]));
			colour += phong(L, N, V, Kd, Ks, shininess, vector_to_vec3(light["color"]));
		}
		else if (light["type"] == "spot") {
			glm::vec3 pos = vector_to_vec3(light["position"]);
			glm::vec3 Dir = normalize(pos - vector_to_vec3(light["direction"]));
			glm::vec3 L = normalize(pos - hitPos);

			float angle = dot(Dir, L);
			if (angle < light["cutoff"]) {
				colour += phong(L, N, V, Kd, Ks, shininess, vector_to_vec3(light["color"]));
			}
		}
	}

	return colour;
}

/****************************************************************************/

bool trace(const point3 &rayOrigin, const point3 &pixel, colour3 &colour, bool pick) {

	// NOTE 1: This is a demo, not ray tracing code! You will need to replace all of this with your own code...
	// NOTE 2: You can work with JSON objects directly (like this sample code), read the JSON objects into your own data structures once and render from those (probably in choose_scene), or hard-code the objects in your own data structures and choose them by name in choose_scene; e.g. choose_scene('e') would pick the same scene as the one in "e.json". Your choice.
	// If you want to use this JSON library, https://github.com/nlohmann/json for more information. The code below gives examples of everything you should need: getting named values, iterating over arrays, and converting types.
	
	glm::vec3 rayDir = pixel - rayOrigin;
	bool didHit = false;

	json& lights = scene["lights"];
	json& objects = scene["objects"];

	// Trying to find the closest object
	json nearestObj;
	glm::vec3 nearestHit;
	glm::vec3 nearestN;
	float rayLen = 999;	
		
	// traverse the objects	
	for (json::iterator it = objects.begin(); it != objects.end(); ++it) {
		json &object = *it;
		
		if (object["type"] == "plane") {
			glm::vec3 a = vector_to_vec3(object["position"]);
			glm::vec3 N = vector_to_vec3(object["normal"]);
			glm::vec3 hitPos;			

			if (isHitPlane(rayOrigin, rayDir, a, N, hitPos)) {
				float thisRayLen = glm::length(hitPos -rayOrigin);
				if (thisRayLen < rayLen) {		
					nearestHit = hitPos; 
					rayLen = thisRayLen;
					nearestN = N;
					nearestObj = object;
				}
				didHit = true;
			}
		}
		else if (object["type"] == "mesh") {
	
			for (std::vector<std::vector<float>> triangle : object["triangles"]) {
				glm::vec3 hitPos;
				glm::vec3 N;
		
				if (isHitTriangle(rayOrigin, rayDir, triangle, hitPos, N)) {
					float thisRayLen = glm::length(hitPos-rayOrigin);
					if (thisRayLen < rayLen) {		
						nearestHit = hitPos;
						rayLen = thisRayLen;		
						nearestN = N;
						nearestObj = object;
					}
					didHit = true;
				}
			}
		}	
		else if (object["type"] == "sphere") {
			glm::vec3 c = vector_to_vec3(object["position"]);
			float r = float(object["radius"]);

			glm::vec3 hitPos;	
			if (isHitSphere(rayOrigin, rayDir, c, r, hitPos)) {
				float thisRayLen = glm::length(hitPos -rayOrigin);
				if (thisRayLen < rayLen) {
					glm::vec3 N = normalize(hitPos - c);		
					nearestN = N;
					nearestHit = hitPos;
					rayLen = thisRayLen;		
					nearestObj = object;
				}
				didHit = true;
			}
		}		
	}
	if (didHit) {
		colour = applyLights(nearestObj, lights, nearestN, pixel, nearestHit);
		return true;
	}

	return false;
}