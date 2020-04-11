// The JSON library allows you to reference JSON arrays like C++ vectors and JSON objects like C++ maps.

#include "raytracer.h"

#include <iostream>
#include <fstream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "json.hpp"

#include <cstdlib>

#include "model.h"

using json = nlohmann::json;

const char *PATH = "scenes/";

json scene;

double fov = 60;

colour3 background_colour(0, 0, 0);
std::vector<Object*> objects;
std::vector<Light*> lights;


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

void jsonImport() {
	json& jsonObjects = scene["objects"];

	// traverse the objects	
	for (json::iterator it = jsonObjects.begin(); it != jsonObjects.end(); ++it) {
		json& object = *it;

		// Material
		json& jsonMaterial = object["material"];
		Material * material = new Material();
		{
			if (jsonMaterial.find("ambient") != jsonMaterial.end()) {
				material->Ka = vector_to_vec3(jsonMaterial["ambient"]);
			}
			if (jsonMaterial.find("diffuse") != jsonMaterial.end()) {
				material->Kd = vector_to_vec3(jsonMaterial["diffuse"]);
			}
			if (jsonMaterial.find("specular") != jsonMaterial.end()) {
				material->Ks = vector_to_vec3(jsonMaterial["specular"]);
			}
			if (jsonMaterial.find("shininess") != jsonMaterial.end()) {
				material->shininess = jsonMaterial["shininess"];
			}
			else {
				material->shininess = 0.0;
			}
			if (jsonMaterial.find("reflective") != jsonMaterial.end()) {
				material->reflection = vector_to_vec3(jsonMaterial["reflective"]);
			}
			if (jsonMaterial.find("transmissive") != jsonMaterial.end()) {
				material->transmission = vector_to_vec3(jsonMaterial["transmissive"]);
			}
			if (jsonMaterial.find("refraction") != jsonMaterial.end()) {
				material->refraction = jsonMaterial["refraction"];
			}
			else {
				material->refraction = 0.0;
			}
		}

		if (object["type"] == "sphere") {
			Sphere * sphere = new Sphere();
			sphere->radius = float(object["radius"]);
			sphere->center = vector_to_vec3(object["position"]);
			sphere->material = material;
			objects.push_back(sphere);
		}

		else if (object["type"] == "plane") {
			Plane * plane = new Plane();
			plane->position = vector_to_vec3(object["position"]);
			plane->normal = normalize(vector_to_vec3(object["normal"]));
			plane->material = material;
			objects.push_back(plane);
		}

		else if (object["type"] == "mesh") {

			std::vector<Triangle*> triangles;

			for (std::vector<std::vector<float>> triangleJson : object["triangles"]) {
				Triangle * triangle = new Triangle();
				triangle->points.push_back(vector_to_vec3(triangleJson[0]));
				triangle->points.push_back(vector_to_vec3(triangleJson[1]));
				triangle->points.push_back(vector_to_vec3(triangleJson[2]));
				triangle->material = material;
				triangles.push_back(triangle);
			}
			Mesh * mesh = new Mesh();
			mesh->triangles = triangles;
			mesh->findSlabs();
			objects.push_back(mesh);
		}
	}

	json& jsonLights = scene["lights"]; 

	// traverse the light sources 
	for (json::iterator it = jsonLights.begin(); it != jsonLights.end(); ++it) {
		json& light = *it;

		if (light["type"] == "ambient") {
			Ambient* ambient = new Ambient();
			ambient->colour = vector_to_vec3(light["color"]);	
			lights.push_back(ambient);
		}
		else if (light["type"] == "point") {
			Point * point = new Point();
			point->colour = vector_to_vec3(light["color"]);
			point->position = vector_to_vec3(light["position"]);
			lights.push_back(point);
		}
		else if (light["type"] == "directional") {
			Directional* directional = new Directional();
			directional->colour = vector_to_vec3(light["color"]);
			directional->direction = normalize(vector_to_vec3(light["direction"]));
			lights.push_back(directional);
		}
		else if (light["type"] == "spot") {
			Spot* spot = new Spot();
			spot->colour = vector_to_vec3(light["color"]);
			spot->direction = normalize(vector_to_vec3(light["direction"])); // oh well..
			spot->position = vector_to_vec3(light["position"]);
			spot->cutoff = float(light["cutoff"]);
			lights.push_back(spot);					
		}
	}
}


/****************************************************************************/


glm::vec3 applyLights(Material * material, 
	glm::vec3 N, glm::vec3 V, glm::vec3 hitPos) {
	
	glm::vec3 colour;

	for (Light * light : lights) {
		colour += light->apply(objects, material, N, V, hitPos);
	}
	return colour;
}

glm::vec3 reflectRay(glm::vec3 hitPos, glm::vec3 N,	glm::vec3 V) {
	return normalize(2 * dot(N, V) * N - V); 
}

glm::vec3 refractRay(glm::vec3 hitPos, glm::vec3 I, glm::vec3 N, float eta, bool inside) {

	float dotIN = dot(I, N);
	float k = 1 - eta * eta * (1 - dotIN * dotIN);

	if (inside) {
		if (k < 0) {
			return glm::vec3(0, 0, 0);
		}
		else {
			return normalize(eta * (I - N * dotIN) - N * sqrt(k));
		}
	}
	else {
		assert(k > 0);		
		return normalize(eta * (I - N * dotIN) - N * sqrt(k));				
	}
	
}

/****************************************************************************/

bool trace(const point3& rayOrigin, const point3& screenPoint, glm::vec3& colour, bool pick) {
	glm::vec3 rayDir = normalize(screenPoint - rayOrigin);

	int bounces = 5;
	colour = trace(rayOrigin, rayDir, bounces, false, pick);
	//colour = trace(rayOrigin, rayDir, bounces, true, pick);

	return colour != background_colour;
}

glm::vec3 trace(glm::vec3 rayOrigin, glm::vec3 rayDir, int bouncesLeft, bool inside, bool pick) {
	glm::vec3 colour(0, 0, 0);
	
	Object * closest = new Object();
	closest->rayLen = MAX_RAY_LEN;

	// traverse the objects	
	for (Object * object : objects) {
		if (object->isHit(rayOrigin, rayDir, MIN_RAY_LEN, MAX_RAY_LEN, inside)) {
			if (object->rayLen < closest->rayLen) {
				closest = object;
			}
		}		
	}
	
	if (closest->rayLen != MAX_RAY_LEN) {

		if (pick) {
			closest->debug();
		}		
	
		glm::vec3 hitPos = rayOrigin + closest->rayLen * rayDir;
		glm::vec3 V = -rayDir;
		glm::vec3 N = closest->normal;				
		Material * material = closest->material;	
	
		if (material->transmission == glm::vec3(0,0,0)) { // absorb everything			
			colour = applyLights(material, N, V, hitPos);
		}
		else { // absorb some portion of a light			

			if (!inside) { // don't want to trap light inside...
				glm::vec3 absorbed = 1.0f - material->transmission;
				colour = absorbed * applyLights(material, N, V, hitPos);
			}				

			if (material->refraction != 0.0) { 				
				// Doesn't work if an object is inside another object... 
				float eta;
				inside == true ? eta = material->refraction : eta = 1.0 / material->refraction;
				glm::vec3 refrDir = refractRay(hitPos, rayDir, N, eta, inside);
				if (refrDir == glm::vec3(0, 0, 0)) {
					if (bouncesLeft > 0) {
						if (pick) {
							printf("Total internal reflection ...\n\n");
						}
						glm::vec3 internalRefl = reflectRay(hitPos, N, V);
						colour += trace(hitPos, internalRefl, bouncesLeft-1, inside, pick);
					}				
				}
				else {
					if (pick) {
						if (!inside) {
							printf("Refracting AIR --> MATERIAL\n\n");
						}
						else {
							printf("Refracting MATERIAL --> AIR\n\n");
						}

					}
					colour += material->transmission * trace(hitPos, refrDir, bouncesLeft, !inside, pick);
				}				
			}	
			else {
				colour += material->transmission * trace(hitPos, rayDir, bouncesLeft, inside, pick);
			}
		}				
		
		if (material->reflection != glm::vec3(0,0,0)) {
			if (bouncesLeft > 0 && !inside) {
				if (pick) {				
					printf("Reflecting ...\n\n");					
				}
				glm::vec3 reflDir = reflectRay(hitPos, N, V);
				colour += material->reflection * trace(hitPos, reflDir, bouncesLeft-1, inside, pick);
			}
		}		
	}
	else {
		colour = background_colour;
	}

	return colour;
}


