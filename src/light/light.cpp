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


glm::vec3 Light::apply(glm::vec3 hitPos, glm::vec3 V, glm::vec3 N, Material* material){
	return glm::vec3(0,0,0);
}


glm::vec3 Ambient::apply(glm::vec3 hitPos, glm::vec3 V, glm::vec3 N, Material* material) {
	return material->Ka * colour;
}


glm::vec3 Directional::apply(glm::vec3 hitPos, glm::vec3 V, glm::vec3 N, Material* material) {

	glm::vec3 L = -direction;

	Ray toLight = Ray();
	toLight.origin = hitPos;
	toLight.direction = L;

	if (!traceShadow(toLight)) {
		return phong(L, N, V, material->Kd, material->Ks, material->shininess, colour);
	}
	else {
		return glm::vec3(0, 0, 0);
	}
}


glm::vec3 Point::apply(glm::vec3 hitPos, glm::vec3 V, glm::vec3 N, Material* material) {
	
	glm::vec3 L = position - hitPos;
	float lightRayLen = length(L);
	L = normalize(L);

	Ray toLight = Ray();
	toLight.origin = hitPos;
	toLight.maxLen = lightRayLen;
	toLight.direction = L;

	if (!traceShadow(toLight)) {
		return phong(L, N, V, material->Kd, material->Ks, material->shininess, colour);
	}
	else {
		return glm::vec3(0, 0, 0);
	}
}


glm::vec3 Spot::apply(glm::vec3 hitPos, glm::vec3 V, glm::vec3 N, Material* material) {

	glm::vec3 L = position - hitPos;
	float lightRayLen = length(L);

	L = normalize(L);

	float dotNL = dot(direction, -L);
	float angle = glm::degrees(acos(dotNL));

	if (angle <= cutoff) {
		Ray toLight = Ray();
		toLight.origin = hitPos;
		toLight.maxLen = lightRayLen;
		toLight.direction = L;

		if (!traceShadow(toLight)) {
			return phong(L, N, V, material->Kd, material->Ks, material->shininess, colour);
		}
	}
	return glm::vec3(0, 0, 0);
}


glm::vec3 Area::apply(glm::vec3 hitPos, glm::vec3 V, glm::vec3 N, Material* material) {

	std::vector<glm::vec3> samples;

	bool prevNoShadow = false;
	int noShadowSequence = 0; 

	for (int i = 0; i < 20; i++) {
		float lenU = distU * (rand() % 100 + 1) / 100.0f;
		float lenV = distV * (rand() % 100 + 1) / 100.0f;

		glm::vec3 currPos = position + dirU * lenU + dirV * lenV;

		glm::vec3 L = currPos - hitPos;
		float lightRayLen = length(L);

		L = normalize(L);

		glm::vec3 incoming;

		Ray ray = Ray();
		ray.origin = hitPos;
		ray.maxLen = lightRayLen;
		ray.direction = L;

		if (!traceShadow(ray)) {
			if (prevNoShadow) {
				noShadowSequence++;
			}			
			prevNoShadow = true;
			if (prevNoShadow > 5) {
				return phong(L, N, V, material->Kd, material->Ks, material->shininess, colour);
			}

			incoming = phong(L, N, V, material->Kd, material->Ks, material->shininess, colour);
		}
		else {			
			prevNoShadow = false;
			noShadowSequence = 0;
			
			incoming = glm::vec3(0, 0, 0);
		}
		samples.push_back(incoming);
	}

	glm::vec3 total;
	for (glm::vec3 curr : samples) {
		total += curr;
	}
	return total / (float) samples.size();	
}


Object* Area:: makeLamp() {
	glm::vec3 v = dirV * distV;
	glm::vec3 u = dirU * distU;
	glm::vec3 o = position;

	Texture* texture = new Texture();
	texture->mode = TextureMode::checkers;
	Material* m = new Material();
	m->Ka = colour;

	Triangle* left = new Triangle();
	left->material = m;
	left->texture = texture;
	left->points.push_back(o);
	left->points.push_back(o + v);
	left->points.push_back(o + v + u);

	Triangle* right = new Triangle();
	right->material = m;
	right->texture = texture;
	right->points.push_back(o);
	right->points.push_back(o + v + u);
	right->points.push_back(o + u);




	Mesh* lamp = new Mesh();
	lamp->triangles.push_back(left);
	lamp->triangles.push_back(right);
	lamp->resetNormals();

	return lamp;
}