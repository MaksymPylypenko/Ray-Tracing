#include "model.h"


bool Triangle::isHit(Ray ray, Hit& hit) {

	if (!hitPlane(ray,hit)) {
		return false;
	}
		
	glm::vec3 hitPos = ray.origin + hit.rayLen * ray.direction;
	float axProj = dot(cross((points[1] - points[0]), (hitPos - points[0])), hit.normal);
	float bxProj = dot(cross((points[2] - points[1]), (hitPos - points[1])), hit.normal);
	float cxProj = dot(cross((points[0] - points[2]), (hitPos - points[2])), hit.normal);
	
	
	if (hit.inside){
		// INSIDE HIT	
		if (axProj <= 0 && bxProj <= 0 && cxProj <= 0) {
			hit.object = this;
			return true;
		}
	}	
	else { 
		// OUTSIDE HIT
		if (axProj >= 0 && bxProj >= 0 && cxProj >= 0) {
			hit.object = this;
			return true;
		}		
	}	
		
	return false;
}


bool Triangle::hitPlane(Ray ray, Hit& hit) {
	// Similar to a infinite plane hit test, but accepting both sides 

	hit.normal = normal;
	glm::vec3 center = points[0];

	float dotND = dot(hit.normal, ray.direction);

	if (dotND < 0) {
		// Normal and Ray pointing in different directions
		hit.inside = false;
	}
	else if (dotND > 0) {
		// Normal and Ray pointing in the same directions
		hit.inside = true;
		dotND = -dotND;
		hit.normal *= -1;
	}
	else { // parallel
		return false;
	}

	hit.rayLen = dot(hit.normal, center - ray.origin) / dotND;

	if (hit.rayLen > ray.minLen&& hit.rayLen < ray.maxLen) {
		return true;
	}
	return false;
}


void Triangle::applyTexture(glm::vec3 hitPos) {
	float u = dot(hitPos, axisU);
	float v = dot(hitPos, axisV);
	material->Ka = texture->getPixel(u, v);
}


void Triangle::setNormal() {
	normal = normalize(cross((points[1] - points[0]), (points[2] - points[0])));

	// For the textures. Simply projecting it from a plane to a triangle.
	// Need to know the longest axis.
	glm::vec3 a = cross(normal, glm::vec3(1, 0, 0));
	glm::vec3 b = cross(normal, glm::vec3(0, 1, 0));
	glm::vec3 c = cross(normal, glm::vec3(0, 0, 1));

	// Note, the dot product of a vector with itself is the square of its magnitude
	axisU = dot(a, a) > dot(b, b) ? a : b;
	axisU = normalize(dot(axisU, axisU) > dot(c, c) ? axisU : c);
	axisV = normalize(cross(normal, axisU));
}


void Triangle::setBarycenter() {
	glm::vec3 curr;
	for (glm::vec3 p : points) {
		curr += p;
	}
	center = curr / 3.0f;
}


void Triangle::toString() {
	printf("Triangle");
}