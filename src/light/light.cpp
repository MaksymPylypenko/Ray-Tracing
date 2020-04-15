#include "light.h"
#include "../raytracer.h"

glm::vec3 phong(glm::vec3 L, glm::vec3 N, glm::vec3 V,
	glm::vec3 Kd, glm::vec3 Ks, float shininess, glm::vec3 Ids)
{
	float dotLN = glm::dot(L, N);
	if (dotLN < 0) { // light does not reach the object					
		Kd = glm::vec3(0.0, 0.0, 0.0);
		Ks = glm::vec3(0.0, 0.0, 0.0);
	}
	else {
		Kd *= dotLN;
		if (Ks != glm::vec3(0.0,0.0,0.0)) { // only calculate this for non-zero specular component
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
	return Kd * Ids + Ks * Ids;
}


glm::vec3 Light::apply(std::vector<Object*> objects, Material* material,
	glm::vec3 N, glm::vec3 V, glm::vec3 hitPos) {
	return glm::vec3(0,0,0);
}


glm::vec3 Ambient::apply(std::vector<Object*> objects, Material* material,
	glm::vec3 N, glm::vec3 V, glm::vec3 hitPos){ 	
	return material->Ka * colour;
}


glm::vec3 Directional::apply(std::vector<Object*> objects, Material* material,
	glm::vec3 N, glm::vec3 V, glm::vec3 hitPos) {

	glm::vec3 L = -direction;

	if (!isShadow(hitPos, L)) {
		return phong(L, N, V, material->Kd, material->Ks, material->shininess, colour);
	}
	else {
		return glm::vec3(0, 0, 0);
	}
}


glm::vec3 Point::apply(std::vector<Object*> objects, Material* material,
	glm::vec3 N, glm::vec3 V, glm::vec3 hitPos) {
	
	glm::vec3 L = position - hitPos;
	float lightRayLen = length(L);
	L = normalize(L);

	if (!isShadow(hitPos, L, lightRayLen)) {
		return phong(L, N, V, material->Kd, material->Ks, material->shininess, colour);
	}
	else {
		return glm::vec3(0, 0, 0);
	}
}


glm::vec3 Spot::apply(std::vector<Object*> objects, Material* material,
	glm::vec3 N, glm::vec3 V, glm::vec3 hitPos) {

	glm::vec3 L = position - hitPos;
	float lightRayLen = length(L);

	L = normalize(L);

	float dotNL = dot(direction, -L);
	float angle = glm::degrees(acos(dotNL));

	if (angle <= cutoff) {
		if (!isShadow(hitPos, L, lightRayLen)) {
			return phong(L, N, V, material->Kd, material->Ks, material->shininess, colour);
		}
	}	
	return glm::vec3(0, 0, 0);	
}
