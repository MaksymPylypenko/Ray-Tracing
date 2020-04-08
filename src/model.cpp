#include "common.h"
#include "model.h"

bool Object::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {
	return false;
}

void Object::debug() {	
	printf("None");	
}

bool Sphere::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {

	float bSq = pow(dot(rayDir, rayOrigin - center), 2);
	float FourAC = dot(rayDir, rayDir) * dot(rayOrigin - center, rayOrigin - center) - radius * radius;
	float discriminant = bSq - FourAC;

	if (discriminant < 0) {
		return false;
	}

	float sqrtDisc = sqrt(discriminant);
	float rest = dot(-rayDir, rayOrigin - center) / dot(rayDir, rayDir);

	if (discriminant > 0) {
		float t1 = rest + sqrtDisc / dot(rayDir, rayDir);
		float t2 = rest - sqrtDisc / dot(rayDir, rayDir);
		if(inside){
			t1 > t2 ? rayLen = t1 : rayLen = t2;
		}
		else {
			t1 < t2 ? rayLen = t1 : rayLen = t2;
		}
	}
	else {  // discriminant < 0 
		rayLen = rest;
	}

	if (rayLen < minRayLen || rayLen > maxRayLen) {
		return false;
	}
		
	normal = normalize((rayOrigin + rayDir * rayLen) - center);
	if (inside) {
		normal = -normal;
	}	
	return true;
}

void Sphere::debug() {
	printf("Sphere @ RayLen = %f\n", rayLen);
}


bool Plane::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {
	

	float dotND = dot(normal, rayDir);
	
	if(dotND < 0){
		rayLen = dot(normal, position - rayOrigin) / dotND;

		if (rayLen <= minRayLen || rayLen > maxRayLen) {
			return false;
		}			
		return true;
	}
	return false;
}

void Plane::debug() {
	printf("Plane @ RayLen = %f\n", rayLen);
}

bool Triangle::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {


	Plane * plane = new Plane();
	plane->position = a;

	glm::vec3 N = normalize(cross((b - a), (c - a)));
	inside == true ? plane->normal = -N : plane->normal = N;
	   

	if (plane->isHit(rayOrigin, rayDir, minRayLen, maxRayLen, inside)) {
		glm::vec3 hitPos = rayOrigin + plane->rayLen * rayDir;

		bool axProj = dot(cross((b - a), (hitPos - a)), plane->normal) < 0;
		bool bxProj = dot(cross((c - b), (hitPos - b)), plane->normal) < 0;
		bool cxProj = dot(cross((a - c), (hitPos - c)), plane->normal) < 0;
		
		bool hit;
		if (inside) {
			hit = axProj && bxProj && cxProj;
		}
		else
		{
			hit = !axProj && !bxProj && !cxProj;
		}

		if (hit) {
			normal = plane->normal;
			rayLen = plane->rayLen;			
			delete plane;
			return true;
		}
	}
	delete plane;
	return false;
}

void Triangle::debug() {
	printf("Triangle @ RayLen = %f\n", rayLen);
}


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


bool isShadow(std::vector<Object*> &objects, glm::vec3 rayOrigin, glm::vec3 rayDir, float maxRayLen = 999) {
	for (Object* object : objects) {
		if (object->isHit(rayOrigin, rayDir, MIN_RAY_LEN, maxRayLen, false)) {
			return true;
		}
	}
	return false;
}


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

	if (!isShadow(objects, hitPos, L)) {
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

	if (!isShadow(objects, hitPos, L, lightRayLen)) {
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
		if (!isShadow(objects,hitPos, L, lightRayLen)) {
			return phong(L, N, V, material->Kd, material->Ks, material->shininess, colour);
		}
	}	
	return glm::vec3(0, 0, 0);	
}
