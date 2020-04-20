#include "raytracer.h"

/// Scene 

SceneAdapter* scene;

void loadScene(char* fn, float& fov, bool& antialiasing) {
	scene = new SceneAdapter();
	scene->chooseScene(fn);
	scene->loadThings();
	fov = scene->fov;
	antialiasing = scene->antialiasing;
}

/// Lights & Shadows

glm::vec3 applyLights(glm::vec3 hitPos, glm::vec3 V, glm::vec3 N, Material* material) {
	
	glm::vec3 colour;

	for (Light * light : scene->lights) {
		colour += light->apply(hitPos,V,N,material);
	}
	return colour;
}


bool traceShadow(Ray ray) {
	ray.closest = false;
	
	// Holds information about the hit.. Can use this to print debug messages
	Hit dummy = Hit(); 

	for (Object* object : scene->objects) {		
		if (object->isHit(ray, dummy)) {
			return true;
		}
	}
	return false;
}



/// Recursive Ray Tracer

glm::vec3 trace(Ray ray) {
		
	glm::vec3 colour(0, 0, 0);			

	Hit currentHit = Hit();
	Hit closestHit = Hit();	

	// traverse the objects	
	for (Object* object : scene->objects) {
		if (object->isHit(ray, currentHit)) {
			if (currentHit.rayLen < closestHit.rayLen) {
				closestHit = currentHit;
			}		
		}		
	}

	if (closestHit.object != nullptr) {
	
		if (ray.debugOn) {
			debug(ray, closestHit);
		}

		Object * obj = closestHit.object;
		glm::vec3 hitPos = ray.origin + closestHit.rayLen * ray.direction;		
			
		/// Union & Intersection & Subtraction of objects
		if (obj->isNegative && !closestHit.inside && !ray.blendingMode) {
			ray.blendingMode = true;
			ray.origin = hitPos;
			if (ray.debugOn) {
				printf("Hitting a negative object ...\n\n");
			}
			return trace(ray);
		}
		else if (obj->isNegative && closestHit.inside && ray.blendingMode) {
			ray.blendingMode = false;
			
			if (!ray.negativeOn) {
				if (ray.debugOn) {
					printf("Leaving a negative object\n\n");
				}
				ray.origin = hitPos;
				return trace(ray);
			}
			if (ray.debugOn) {
				printf("Rendering a inner part of a negative object\n\n");
			}			
		}
		else if (!obj->isNegative && ray.blendingMode) {

			if (ray.debugOn) {
				printf("Hitting a positive object\n\n");
			}
			ray.origin = hitPos;
			ray.negativeOn = !ray.negativeOn;
			return trace(ray);
		}
		
		// Normal trace routine ...

		if (obj->texture->mode != TextureMode::none) {
			obj->applyTexture(hitPos);
		}
		

		glm::vec3 V = -ray.direction;
		glm::vec3 N = closestHit.normal;
		Material * material = obj->material;

		if (material->transmission == glm::vec3(0,0,0)) { // absorb everything			
			colour = applyLights(hitPos, V, N, material);
		}
		else { // absorb some portion of a light			

			if (closestHit.inside) { // don't want to trap light inside...
				glm::vec3 absorbed = 1.0f - material->transmission;
				colour = absorbed * applyLights(hitPos, V, N, material);
			}				

			if (material->refraction != 0.0) { 				
				// Doesn't work if an object is inside another object... 
				float eta = material->refraction;

				if (ray.refract(hitPos, N, eta, closestHit.inside)) {
					if (ray.debugOn) {
						if (closestHit.inside) {
							printf("Refracting MATERIAL --> AIR\n\n");
						}
						else {
							printf("Refracting AIR --> MATERIAL\n\n");
						}
					}
					ray.origin = ray.origin;
					colour += material->transmission * trace(ray);					
				}
				else {
					if (ray.bouncesLeft > 0) {
						if (ray.debugOn) {
							printf("Total internal reflection ...\n\n");
						}
						ray.reflect(hitPos, N, V);
						ray.bouncesLeft--;
						colour += trace(ray);
					}
				}				
			}	
			else {
				if (ray.debugOn) {
					printf("Simple Transmission ...\n\n");
				}
				ray.origin = hitPos;
				colour += material->transmission * trace(ray);
			}
		}				
		
		if (material->reflection != glm::vec3(0,0,0)) {
			if (ray.bouncesLeft > 0 ) { //&& !closest->inside) {
				if (ray.debugOn) {
					printf("Reflecting ...\n\n");					
				}
				ray.reflect(hitPos, N, V);
				ray.bouncesLeft--;
				colour += material->reflection * trace(ray);
			}
		}		
	}
	else {
		colour = scene->background_colour;
	}

	return colour;
}


