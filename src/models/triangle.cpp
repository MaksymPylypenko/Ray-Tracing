#include "model.h"

void Triangle::setNormal() {
	fixedNormal = normalize(cross((points[1] - points[0]), (points[2] - points[0])));
}

bool Triangle::hitPlane(Ray ray) {
	// Similar to a infinite plane hit test, but accepting both sides 

	normal = fixedNormal;
	glm::vec3 center = points[0];

	float dotND = dot(normal, ray.direction);

	if (dotND < 0) {
		// Normal and Ray pointing in different directions
		inside = false;		
	}
	else if (dotND > 0) {
		// Normal and Ray pointing in the same directions
		inside = true;
		dotND = -dotND;
		normal = -normal;		
	}
	else { // parallel
		return false;
	}

	rayLen = dot(normal, center - ray.origin) / dotND;

	if (rayLen > ray.minLen && rayLen < ray.maxLen) {
		return true;
	}
	return false;
}


bool Triangle::isHit(Ray ray) {

	if (!hitPlane(ray)) {
		return false;
	}
		
	glm::vec3 hitPos = ray.origin + rayLen * ray.direction;
	float axProj = dot(cross((points[1] - points[0]), (hitPos - points[0])), normal);
	float bxProj = dot(cross((points[2] - points[1]), (hitPos - points[1])), normal);
	float cxProj = dot(cross((points[0] - points[2]), (hitPos - points[2])), normal);
	
	
	if (inside){ 	
		// INSIDE HIT	
		if (axProj <= 0 && bxProj <= 0 && cxProj <= 0) {
			return true;
		}
	}	
	else { 
		// OUTSIDE HIT
		if (axProj >= 0 && bxProj >= 0 && cxProj >= 0) {
			return true;
		}		
	}	
		
	return false;
}


void Triangle::applyTexture(glm::vec3 hitPos) {

	//Project to a plane
	glm::vec3 axisU = normalize(glm::vec3(normal.y, normal.z, -normal.x));
	glm::vec3 axisV = normalize(cross(normal, axisU));
	float u = dot(hitPos, axisU);
	float v = dot(hitPos, axisV);
	material->Ka = texture->getPixel(u, v);
}


void Triangle::setBarycenter() {
	glm::vec3 curr;
	for (glm::vec3 p : points) {
		curr += p;
	}
	center = curr / 3.0f;
}

void Triangle::debug() {
	printf("Triangle HIT %s @ RayLen = %f, \n", inside ? "from [Inside]" : "from [Outside]", rayLen);
}