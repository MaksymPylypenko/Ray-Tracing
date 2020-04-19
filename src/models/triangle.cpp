#include "model.h"


bool Triangle::isHit(Ray ray) {
	
	Plane* plane = new Plane();
	plane->center = points[0];
	plane->normal = -normalize(cross((points[1] - points[0]), (points[2] - points[0])));

	if (texture->mode != TextureMode::none) {
		plane->material = new Material();
		plane->texture = texture;
		plane->alignTextureAxes(); // @TODO, decouple this
	}	

	if (plane->isHit(ray)) {
		glm::vec3 hitPos = ray.origin + plane->rayLen * ray.direction;

		float axProj = dot(cross((points[1] - points[0]), (hitPos - points[0])), plane->normal);
		float bxProj = dot(cross((points[2] - points[1]), (hitPos - points[1])), plane->normal);
		float cxProj = dot(cross((points[0] - points[2]), (hitPos - points[2])), plane->normal);

		bool hit = false;
	
		if (plane->inside) {
			// INSIDE HIT
			if (axProj <= 0 && bxProj <= 0 && cxProj <= 0) {
				normal = plane->normal;
				inside = true;
				hit = true;
			}			
		}
		else {
			// OUTSIDE HIT
			if (axProj >= 0 && bxProj >= 0 && cxProj >= 0)
			{
				normal = plane->normal;
				inside = false;
				hit = true;
			}
		}

		if (hit) {
			if (texture->mode != TextureMode::none) {
				// projecting plane texture onto triangle
				material->Ka = plane->material->Ka;
			}
			
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
	printf("Triangle HIT %s @ RayLen = %f, \n", inside ? "from [Inside]" : "from [Outside]", rayLen);
}