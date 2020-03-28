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

bool isHitSphere(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 sphereCenter, 
	float radius, float &rayLen, float maxRayLen = 999) {

	float bSq = pow(dot(rayDir, rayOrigin - sphereCenter), 2);
	float FourAC = dot(rayDir, rayDir) * dot(rayOrigin - sphereCenter, rayOrigin - sphereCenter) - radius * radius;
	float discriminant = bSq - FourAC;

	if (discriminant < 0) {
		return false;
	}

	float sqrtDisc = sqrt(discriminant);
	float rest = dot(-rayDir, rayOrigin - sphereCenter) / dot(rayDir, rayDir);
	
	if (discriminant > 0) {
		float t1 = rest + sqrtDisc / dot(rayDir, rayDir);
		float t2 = rest - sqrtDisc / dot(rayDir, rayDir);
		t1 < t2 ? rayLen = t1: rayLen = t2;		
	}
	else {  // discriminant < 0 
		rayLen = rest;
	}

	if (rayLen < 0.001 || rayLen > maxRayLen) {
		return false;
	}
	return true;
}


/****************************************************************************/

bool isHitPlane(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 planePos, 
	glm::vec3 N, float &rayLen, float maxRayLen = 999) {

	float dotND = dot(N, rayDir); // sign tells us which side of a plane was hit
	if (dotND < 0) {
		rayLen = dot(N, planePos - rayOrigin) / dotND;

		if (rayLen < 0.001 || rayLen > maxRayLen) {
			return false;
		}
		return true;		
	}	
	return false;
}


/****************************************************************************/

//bool isHitMesh(glm::vec3 rayOrigin, glm::vec3 rayDir, 
//	std::vector<std::vector<std::vector<float>>> mesh, glm::vec3& hit) {
//
//	float minX = mesh[0][0][0]; // first triangle, first point, x
//	float minY = mesh[0][0][1];
//	float minZ = mesh[0][0][2];
//	float maxX = mesh[0][0][0];
//	float maxY = mesh[0][0][1];
//	float maxZ = mesh[0][0][2];
//
//	for (std::vector<std::vector<float>> triangle :mesh) {
//		for (std::vector<float> point : triangle) {
//			glm::vec3 curr = vector_to_vec3(point);
//			minX = (curr.x < minX ? curr.x : minX);
//			minY = (curr.y < minY ? curr.y : minY);
//			minZ = (curr.z < minZ ? curr.z : minZ);
//			maxX = (curr.x > maxX ? curr.x : maxX);
//			maxY = (curr.y > maxY ? curr.y : maxY);
//			maxZ = (curr.z > maxZ ? curr.z : maxZ);			
//		}
//	}
//
//	// @TODO return true if inside the bounding box
//
//	return false;
//}


bool isHitTriangle(glm::vec3 rayOrigin, glm::vec3 rayDir,
	std::vector<std::vector<float>> triangle, glm::vec3& N, float& rayLen, float maxRayLen = 999) {

	glm::vec3 a = vector_to_vec3(triangle[0]);
	glm::vec3 b = vector_to_vec3(triangle[1]);
	glm::vec3 c = vector_to_vec3(triangle[2]);

	N = normalize(cross((b - a),(c - a)));
	bool isPlaneHit = isHitPlane(rayOrigin, rayDir, a, N, rayLen, maxRayLen);
	glm::vec3 hitPos = rayOrigin + rayLen * rayDir;

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

bool isShadow(glm::vec3 rayOrigin, glm::vec3 rayDir, float maxRayLen = 999) {
	json& objects = scene["objects"];
	   
	// traverse the objects	
	for (json::iterator it = objects.begin(); it != objects.end(); ++it) {
		json& object = *it;

		if (object["type"] == "plane") {
			glm::vec3 a = vector_to_vec3(object["position"]);
			glm::vec3 N = vector_to_vec3(object["normal"]);

			float planeRayLen;
			if (isHitPlane(rayOrigin, rayDir, a, N, planeRayLen, maxRayLen)) {				
				return true;
			}
		}

		else if (object["type"] == "mesh") {
			for (std::vector<std::vector<float>> triangle : object["triangles"]) {
				glm::vec3 N; // not used
				float rayLen; // not used
				if (isHitTriangle(rayOrigin, rayDir, triangle, N, rayLen, maxRayLen)) {
					return true;
				}
			}
		}
		else if (object["type"] == "sphere") {
			glm::vec3 c = vector_to_vec3(object["position"]);
			float r = float(object["radius"]);
			float rayLen; // not used
			if (isHitSphere(rayOrigin, rayDir, c, r, rayLen, maxRayLen)) {
				return true;
			}
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
			glm::vec3 R = normalize(2 * dotLN * N - L); 
			float dotRV = dot(R, V);
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
	glm::vec3 N, glm::vec3 V, glm::vec3 hitPos) {
	
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
			glm::vec3 L = vector_to_vec3(light["position"]) - hitPos;
			float lightRayLen = length(L);
			L = normalize(L);

			if (!isShadow(hitPos, L, lightRayLen)) {
				colour += phong(L, N, V, Kd, Ks, shininess, vector_to_vec3(light["color"]));
			}			
		}
		else if (light["type"] == "directional") {
			glm::vec3 L = -vector_to_vec3(light["direction"]);
			if (!isShadow(hitPos, L)) {
				colour += phong(L, N, V, Kd, Ks, shininess, vector_to_vec3(light["color"]));
			}
		}
		else if (light["type"] == "spot") {		
			glm::vec3 L = vector_to_vec3(light["position"]) - hitPos;
			float lightRayLen = length(L);
			L = normalize(L);
			glm::vec3 Dir = normalize(vector_to_vec3(light["direction"]));
			float angle = glm::degrees(acos(dot(Dir, -L)));
			if (angle <= light["cutoff"]) {
				if (!isShadow(hitPos, L, lightRayLen)) {
					colour += phong(L, N, V, Kd, Ks, shininess, vector_to_vec3(light["color"]));
				}
			}
		}
	}

	return colour;
}

/****************************************************************************/

bool trace(const point3& rayOrigin, const point3& screenPoint, colour3& colour, bool pick) {
	glm::vec3 rayDir = screenPoint - rayOrigin;

	int bounces = 5;
	return trace(rayOrigin, rayDir, colour, bounces);
}

bool trace(glm::vec3 rayOrigin, glm::vec3 rayDir, colour3& colour, int bouncesLeft, float refrIndex) {
	bool didHit = false;

	json& lights = scene["lights"];
	json& objects = scene["objects"];

	// Trying to find the closest object
	float rayLen = 999;
	json hitObj;
	glm::vec3 hitNormal;	

	// traverse the objects	
	for (json::iterator it = objects.begin(); it != objects.end(); ++it) {
		json& object = *it;

		if (object["type"] == "plane") {
			glm::vec3 a = vector_to_vec3(object["position"]);
			glm::vec3 N = normalize(vector_to_vec3(object["normal"]));			
			float length;

			if (isHitPlane(rayOrigin, rayDir, a, N, length)) {
				if (length < rayLen) {
					rayLen = length;
					hitNormal = N;
					hitObj = object;
				}
				didHit = true;
			}
		}
		else if (object["type"] == "mesh") {
			for (std::vector<std::vector<float>> triangle : object["triangles"]) {				
				glm::vec3 N;
				float length;

				if (isHitTriangle(rayOrigin, rayDir, triangle, N, length)) {					
					if (length < rayLen) {
						rayLen = length;
						hitNormal = N;
						hitObj = object;
					}
					didHit = true;
				}
			}
		}
		else if (object["type"] == "sphere") {
			glm::vec3 c = vector_to_vec3(object["position"]);
			float r = float(object["radius"]);
			float t;
			float length;
			if (isHitSphere(rayOrigin, rayDir, c, r, length)) {				
				if (length < rayLen) {
					rayLen = length;
					glm::vec3 hit = rayOrigin + rayLen * rayDir;
					hitNormal = normalize(hit - c);
					hitObj = object;
				}
				didHit = true;
			}
		}
	}
	if (didHit) {
		glm::vec3 hitPos = rayOrigin + rayLen * rayDir;
		glm::vec3 V = -normalize(rayDir);
		
		json& material = hitObj["material"];
			   		
		if (material.find("transmissive") != material.end()) {
			// some lights goes through the object
			glm::vec3 transmitted = vector_to_vec3(material["transmissive"]);
			glm::vec3 absorbed = glm::vec3(1, 1, 1) - transmitted;
			colour = absorbed * applyLights(hitObj, lights, hitNormal, V, hitPos);
					   
			if (material.find("refraction") != material.end()) {
				float refrIndex2 = material["refraction"];

				// Are we entering the object or leaving it?
				if (refrIndex != 1) {
					refrIndex2 = 1; // not correct for objects inside other objects
				}

				float refrIndexSq = refrIndex * refrIndex;
				float refrIndex2Sq = refrIndex2 * refrIndex2;
				float dotVN = dot(V, hitNormal);

				glm::vec3 Vr = refrIndex * (V - hitNormal * dotVN) / refrIndex2;
				Vr -= hitNormal * sqrt(1 - refrIndexSq * dotVN* dotVN / refrIndex2Sq);

				rayDir = Vr;
				refrIndex = refrIndex2;
			}	

			glm::vec3 tColour;
			if (trace(hitPos, rayDir, tColour, bouncesLeft, refrIndex)) {
				colour += transmitted * tColour;
			}			
		}
		else { // absorp everything
			colour = applyLights(hitObj, lights, hitNormal, V, hitPos);
		}

		
		if (material.find("reflective") != material.end()) {
			// bounce a ray and return some % of the final color
			if (bouncesLeft > 0) {
				glm::vec3 reflected = vector_to_vec3(material["reflective"]);	
				glm::vec3 R = normalize(2 * dot(hitNormal, V) * hitNormal - V);
				glm::vec3 rColour;
				if (trace(hitPos, R, rColour, bouncesLeft - 1, refrIndex)) {
					colour += reflected * rColour;
				}
			}
		}
		
		return true;
	}

	return false;
}


