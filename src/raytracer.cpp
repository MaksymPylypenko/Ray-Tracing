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

bool isShadow(Ray ray) {
	for (Object* object : scene->objects) {
		if (object->isHit(ray)) {
			return true;
		}
	}
	return false;
}



/// Recursive Ray Tracer

glm::vec3 trace(Ray ray, bool showDebug) {
		
	glm::vec3 colour(0, 0, 0);			

	Object* closest = new Object();
	closest->rayLen = ray.maxLen;

	// traverse the objects	
	for (Object* object : scene->objects) {
		if (object->isHit(ray)) {
			if (object->rayLen < closest->rayLen) {
				closest = object;
			}
		}
	}

	if (closest->rayLen != ray.maxLen) {
				
		if (showDebug) {
			closest->debug();
		}

		glm::vec3 hitPos = ray.origin + closest->rayLen * ray.direction;
		bool inside = closest->inside;
			
		/// Union & Intersection & Subtraction of objects
		if (closest->isNegative && !inside && !ray.blendingMode) {
			ray.blendingMode = true;
			ray.origin = hitPos;
			if (showDebug) {
				printf("Hitting a negative object ...\n\n");
			}
			return trace(ray, showDebug);
		}
		else if (closest->isNegative && inside && ray.blendingMode) {
			ray.blendingMode = false;
			
			if (!ray.negativeOn) {
				if (showDebug) {
					printf("Leaving a negative object\n\n");
				}
				ray.origin = hitPos;
				return trace(ray, showDebug);
			}
			if (showDebug) {
				printf("Rendering a inner part of a negative object\n\n");
			}			
		}
		else if (!closest->isNegative && ray.blendingMode) {

			if (showDebug) {
				printf("Hitting a positive object\n\n");
			}
			ray.origin = hitPos;
			ray.negativeOn = !ray.negativeOn;
			return trace(ray, showDebug);
		}
		
		// Normal trace routine ...

		if (closest->texture->mode != TextureMode::none) {
			closest->applyTexture(hitPos);
		}
		
		glm::vec3 V = -ray.direction;
		glm::vec3 N = closest->normal;
		Material * material = closest->material;

		if (material->transmission == glm::vec3(0,0,0)) { // absorb everything			
			colour = applyLights(material, N, V, hitPos);
		}
		else { // absorb some portion of a light			

			if (inside) { // don't want to trap light inside...
				glm::vec3 absorbed = 1.0f - material->transmission;
				colour = absorbed * applyLights(material, N, V, hitPos);
			}				

			if (material->refraction != 0.0) { 				
				// Doesn't work if an object is inside another object... 
				float eta = material->refraction;

				if (ray.refract(hitPos, N, eta, inside)) {
					if (showDebug) {
						if (inside) {
							printf("Refracting MATERIAL --> AIR\n\n");
						}
						else {
							printf("Refracting AIR --> MATERIAL\n\n");
						}
					}
					ray.origin = ray.origin;
					colour += material->transmission * trace(ray, showDebug);					
				}
				else {
					if (ray.bouncesLeft > 0) {
						if (showDebug) {
							printf("Total internal reflection ...\n\n");
						}
						ray.reflect(hitPos, N, V);
						ray.bouncesLeft--;
						colour += trace(ray, showDebug);
					}
				}				
			}	
			else {
				if (showDebug) {
					printf("Simple Transmission ...\n\n");
				}
				ray.origin = hitPos;
				colour += material->transmission * trace(ray, showDebug);
			}
		}				
		
		if (material->reflection != glm::vec3(0,0,0)) {
			if (ray.bouncesLeft > 0 ) { //&& !closest->inside) {
				if (showDebug) {
					printf("Reflecting ...\n\n");					
				}
				ray.reflect(hitPos, N, V);
				ray.bouncesLeft--;
				colour += material->reflection * trace(ray, showDebug);
			}
		}		
	}
	else {
		colour = scene->background_colour;
	}

	return colour;
}


