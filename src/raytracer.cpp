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

glm::vec3 trace(Ray ray, bool negativeHasColour, bool showDebug) {
		
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

		//// Hitting a negative object
		//if (closest->isNegative && !negativeHasColour) {
		//	ray.origin = hitPos;
		//	ray.isInside = !ray.isInside;
		//	return trace(ray, negativeHasColour, showDebug);
		//}
		//// Flip bool, whenq hitting a positive objct 
		//if (!closest->isNegative) {
		//	return trace(hitPos, rayDir, bouncesLeft, !isInside, !negativeHasColour, showDebug);
		//}


		
		glm::vec3 V = -ray.direction;
		glm::vec3 N = closest->normal;
		Material * material = closest->material;

		if (material->transmission == glm::vec3(0,0,0)) { // absorb everything			
			colour = applyLights(material, N, V, hitPos);
		}
		else { // absorb some portion of a light			

			if (!ray.isInside) { // don't want to trap light inside...
				glm::vec3 absorbed = 1.0f - material->transmission;
				colour = absorbed * applyLights(material, N, V, hitPos);
			}				

			if (material->refraction != 0.0) { 				
				// Doesn't work if an object is inside another object... 
				float eta;
				ray.isInside == true ? eta = material->refraction : eta = 1.0 / material->refraction;
				ray.refract(hitPos, N, eta);
				if (ray.direction == glm::vec3(0, 0, 0)) {
					if (ray.bouncesLeft > 0) {
						if (showDebug) {
							printf("Total internal reflection ...\n\n");
						}
						ray.reflect(hitPos, N, V);
						ray.bouncesLeft--;
						colour += trace(ray, negativeHasColour, showDebug);
					}				
				}
				else {
					if (showDebug) {
						if (!ray.isInside) {
							printf("Refracting AIR --> MATERIAL\n\n");
						}
						else {
							printf("Refracting MATERIAL --> AIR\n\n");
						}

					}
					ray.isInside = !ray.isInside;
					colour += material->transmission * trace(ray, negativeHasColour, showDebug);
				}				
			}	
			else {
				colour += material->transmission * trace(ray, negativeHasColour, showDebug);
			}
		}				
		
		if (material->reflection != glm::vec3(0,0,0)) {
			if (ray.bouncesLeft > 0 && !ray.isInside) {
				if (showDebug) {
					printf("Reflecting ...\n\n");					
				}
				ray.reflect(hitPos, N, V);
				ray.bouncesLeft--;
				colour += material->reflection * trace(ray, negativeHasColour, showDebug);
			}
		}		
	}
	else {
		colour = scene->background_colour;
	}

	return colour;
}


