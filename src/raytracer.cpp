#include "raytracer.h"

/// Scene 

SceneAdapter* scene;

float loadScene(char* fn) {
	scene = new SceneAdapter();
	scene->choose_scene(fn);
	scene->jsonImport();
	return scene->fov;
}

/// Lights & Shadows

glm::vec3 applyLights(Material * material, 
	glm::vec3 N, glm::vec3 V, glm::vec3 hitPos) {
	
	glm::vec3 colour;

	for (Light * light : scene->lights) {
		colour += light->apply(scene->objects, material, N, V, hitPos);
	}
	return colour;
}

bool isShadow(glm::vec3 rayOrigin, glm::vec3 rayDir, float maxRayLen) {
	for (Object* object : scene->objects) {
		if (object->isHit(rayOrigin, rayDir, MIN_RAY_LEN, maxRayLen, false)) {
			return true;
		}
	}
	return false;
}

/// Change of direction

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

/// Recursive Ray Tracer

glm::vec3 trace(glm::vec3 rayOrigin, glm::vec3 rayDir, int bouncesLeft,
	bool isInside, bool showDebug) {
		
	glm::vec3 colour(0, 0, 0);
			
	if(scene->bvHierarchy->isHit(rayOrigin, rayDir, MIN_RAY_LEN, MAX_RAY_LEN, isInside))
	{
		Object* closest = scene->bvHierarchy->closest;

		if (showDebug) {
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

			if (!isInside) { // don't want to trap light inside...
				glm::vec3 absorbed = 1.0f - material->transmission;
				colour = absorbed * applyLights(material, N, V, hitPos);
			}				

			if (material->refraction != 0.0) { 				
				// Doesn't work if an object is inside another object... 
				float eta;
				isInside == true ? eta = material->refraction : eta = 1.0 / material->refraction;
				glm::vec3 refrDir = refractRay(hitPos, rayDir, N, eta, isInside);
				if (refrDir == glm::vec3(0, 0, 0)) {
					if (bouncesLeft > 0) {
						if (showDebug) {
							printf("Total internal reflection ...\n\n");
						}
						glm::vec3 internalRefl = reflectRay(hitPos, N, V);
						colour += trace(hitPos, internalRefl, bouncesLeft-1, isInside, showDebug);
					}				
				}
				else {
					if (showDebug) {
						if (!isInside) {
							printf("Refracting AIR --> MATERIAL\n\n");
						}
						else {
							printf("Refracting MATERIAL --> AIR\n\n");
						}

					}
					colour += material->transmission * trace(hitPos, refrDir, bouncesLeft, !isInside, showDebug);
				}				
			}	
			else {
				colour += material->transmission * trace(hitPos, rayDir, bouncesLeft, isInside, showDebug);
			}
		}				
		
		if (material->reflection != glm::vec3(0,0,0)) {
			if (bouncesLeft > 0 && !isInside) {
				if (showDebug) {
					printf("Reflecting ...\n\n");					
				}
				glm::vec3 reflDir = reflectRay(hitPos, N, V);
				colour += material->reflection * trace(hitPos, reflDir, bouncesLeft-1, isInside, showDebug);
			}
		}		
	}
	else {
		colour = scene->background_colour;
	}

	return colour;
}


