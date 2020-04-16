#include "model.h"


bool Triangle::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {


	Plane* plane = new Plane();
	plane->center = points[0];

	glm::vec3 N = normalize(cross((points[1] - points[0]), (points[2] - points[0])));
	inside == true ? plane->normal = -N : plane->normal = N;

	if (texture) {
		plane->material = new Material();
		plane->texture = true;
		plane->alignTextureAxes();
		plane->scale = 10.0;
	}
	

	if (plane->isHit(rayOrigin, rayDir, minRayLen, maxRayLen, inside)) {
		glm::vec3 hitPos = rayOrigin + plane->rayLen * rayDir;

		float axProj = dot(cross((points[1] - points[0]), (hitPos - points[0])), plane->normal);
		float bxProj = dot(cross((points[2] - points[1]), (hitPos - points[1])), plane->normal);
		float cxProj = dot(cross((points[0] - points[2]), (hitPos - points[2])), plane->normal);

		bool hit;
		if (inside) {
			hit = axProj < 0 && bxProj < 0 && cxProj < 0;
		}
		else
		{
			hit = axProj > 0 && bxProj > 0 && cxProj > 0;
		}

		if (hit) {
			if (texture) {
				material->Ka = plane->material->Ka;
			}
			normal = plane->normal;
			rayLen = plane->rayLen;
			delete plane;
			return true;
		}
	}
	delete plane;
	return false;
}

void Triangle::setBarycenter() {
	glm::vec3 curr;
	for (glm::vec3 p : points) {
		curr += p;
	}
	center = curr / 3.0f;
}

void Triangle::debug() {
	printf("Triangle @ RayLen = %f\n", rayLen);
}