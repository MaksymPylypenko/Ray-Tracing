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
				material->medium = jsonMaterial["refraction"];
			}
			else {
				material->medium = 1.0;
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
			for (std::vector<std::vector<float>> triangleJson : object["triangles"]) {
				Triangle * triangle = new Triangle();
				triangle->a = vector_to_vec3(triangleJson[0]);
				triangle->b = vector_to_vec3(triangleJson[1]);
				triangle->c = vector_to_vec3(triangleJson[2]);
				triangle->material = material;
				objects.push_back(triangle);
			}

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
			directional->direction = vector_to_vec3(light["direction"]);	
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

glm::vec3 reflectRay(glm::vec3 hitPos, glm::vec3 N,
	glm::vec3 V, int bouncesLeft, float medium) {
	if (bouncesLeft > 0) {
		//N = normalize(N);
		glm::vec3 R = normalize(2 * dot(N, V) * N - V);	
		return trace(hitPos, R, bouncesLeft - 1, medium);		
	}
	return glm::vec3(0, 0, 0);
}


/****************************************************************************/

bool trace(const point3& rayOrigin, const point3& screenPoint, glm::vec3& colour, bool pick) {
	glm::vec3 rayDir = screenPoint - rayOrigin;

	int bounces = 5;
	float vacuum = 1.0;
	colour = trace(rayOrigin, rayDir, bounces, vacuum);

	return colour != background_colour;
}

glm::vec3 trace(glm::vec3 rayOrigin, glm::vec3 rayDir, int bouncesLeft, float medium) {
	glm::vec3 colour(0, 0, 0);

	json& lights = scene["lights"];

	Object * closest = new Object();
	closest->rayLen = 999;

	// traverse the objects	
	for (Object * object : objects) {
		if (object->isHit(rayOrigin, rayDir, 0.0001, 999)) {
			if (object->rayLen < closest->rayLen) {
				closest = object;
			}
		}		
	}
	
	if (closest->rayLen != 999) {
		glm::vec3 hitPos = rayOrigin + closest->rayLen * rayDir;
		glm::vec3 V = -normalize(rayDir);
		glm::vec3 N = normalize(closest->normal);
		
		Material * material = closest->material;	
	
		if (material->transmission == glm::vec3(0,0,0)) { // absorb everything			
			colour = applyLights(material, N, V, hitPos);
		}
		else { // absorb some portion of a light		
			
			glm::vec3 absorbed = glm::vec3(1, 1, 1) - material->transmission;
			colour = absorbed * applyLights(material, N, V, hitPos);

			if (material->medium == 1.0) {  // cast a ray in the same direction				
				colour += material->transmission * trace(hitPos, rayDir, bouncesLeft, medium);
			}
			else { // cast a ray in a different direction	
				float medium2 = material->medium;

				if (medium == medium2) { // we are inside an object
					medium2 = 1.0; // assume that the outside medium is vacum
					N = -N;
					// not correct for objects inside other objects
					// not correct for reflections inside an object 
					// not correct for planes...
				}

				float coef = medium / medium2;
				float coefSq = coef * coef;
				float dotVN = dot(V, N);
				float dotVNSq = dotVN * dotVN;
				float insideSqRoot = 1 - coefSq * (1 - dotVNSq);

				if (insideSqRoot < 0) { // total internal reflection
					colour += material->transmission * reflectRay(hitPos, N, V, bouncesLeft, medium);
				}
				else { // refraction
					glm::vec3 Vr = coef * (V - N * dotVN) - N * sqrt(insideSqRoot);
					colour += material->transmission * trace(hitPos, Vr, bouncesLeft, medium2);
				}
			}
		}					
		
		if (material->reflection != glm::vec3(0,0,0)) {
			colour += material->reflection * reflectRay(hitPos, N,V,bouncesLeft,medium);
		}		
	}
	else {
		colour = background_colour;
	}

	return colour;
}


