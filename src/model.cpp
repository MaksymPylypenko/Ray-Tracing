#include "common.h"
#include "model.h"

/*************************************************************************/
/* Objects */

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

	// @TODO: decouple the texture thing..
	// Math: https://people.cs.clemson.edu/~dhouse/courses/405/notes/texture-maps.pdf

	glm::vec3 hitPos = rayOrigin + rayDir * rayLen;
	glm::vec3 point = hitPos- center;
	float scale = 12.0;

	// polar angle
	float phi = acosf(point.y / radius);

	// azimuthal angle
	float theta = atan2f(point.x, point.z);

	// The direction and proportion is not correct when reading texture from a regular bmp file!
	// @TODO See this for improvement http://www.raytracerchallenge.com/bonus/texture-mapping.html
	float u = theta / 3.1415926f;
	float v = phi / 3.1415926f;	

	bool white = (int(round(u * scale)) + int(round(v * scale))) % 2 == 0;

	glm::vec3 colour;
	if (white) {
		colour = glm::vec3(1.0, 1.0, 1.0);
	}
	else {
		colour = glm::vec3(0.1, 0.1, 0.1);
	}
	material->Ka = colour;


	return true;
}

void Sphere::debug() {
	printf("Sphere @ RayLen = %f\n", rayLen);
}


void Plane::alignTextureAxes() {	
	// This caused issues with a vertical plane on scene [c]...
	// axisU = normalize(glm::vec3(normal.y, normal.z, -normal.x));

	// Instead, setting [axisU] to the longest axis
	glm::vec3 a = cross(normal, glm::vec3(1, 0, 0));
	glm::vec3 b = cross(normal, glm::vec3(0, 1, 0));
	glm::vec3 c = cross(normal, glm::vec3(0, 0, 1));

	// Note, the dot product of a vector with itself is the square of its magnitude
	axisU = dot(a, a) > dot(b, b) ? a : b;
	axisU = normalize(dot(axisU, axisU) > dot(c, c) ? axisU : c);	   	
	axisV = normalize(cross(normal, axisU));
	allowTexture = true;
}

bool Plane::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {
	

	float dotND = dot(normal, rayDir);
	
	if(dotND < 0){
		rayLen = dot(normal, position - rayOrigin) / dotND;

		if (rayLen <= minRayLen || rayLen > maxRayLen) {
			return false;
		}			
			   
		if (allowTexture) {
			glm::vec3 hitPos = rayOrigin + rayDir * rayLen;
			float u = dot(hitPos, axisU);
			float v = dot(hitPos, axisV);

			float scale = 1.68;
			bool white = (int(round(u * scale)) + int(round(v * scale))) % 2 == 0;

			glm::vec3 colour;

			if (white) {
				colour = glm::vec3(1.0,1.0,1.0);
			}
			else {
				colour = glm::vec3(0.4, 0.4, 0.4);
			}	
			material->Ka = colour;
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
	plane->position = points[0];

	glm::vec3 N = normalize(cross((points[1] - points[0]), (points[2] - points[0])));
	inside == true ? plane->normal = -N : plane->normal = N;
	   
	if (plane->isHit(rayOrigin, rayDir, minRayLen, maxRayLen, inside)) {
		glm::vec3 hitPos = rayOrigin + plane->rayLen * rayDir;

		float axProj = dot(cross((points[1] - points[0]), (hitPos - points[0])), plane->normal);
		float bxProj = dot(cross((points[2] - points[1]), (hitPos - points[1])), plane->normal);
		float cxProj = dot(cross((points[0] - points[2]), (hitPos - points[2])), plane->normal);
		
		bool hit;
		if (inside) {
			hit = axProj<0 && bxProj<0 && cxProj<0;
		}
		else
		{
			hit = axProj>0 && bxProj>0 && cxProj>0;
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

glm::vec3 Triangle::getBarycenter() {
	glm::vec3 curr;
	for (glm::vec3 p : points) {
		curr += p;
	}
	return curr / (float) points.size();
}

void Triangle::debug() {
	printf("Triangle @ RayLen = %f\n", rayLen);
}


void Mesh:: findSlabs() {
	// init slabs to the first point in the mesh
	min = triangles[0]->points[0];
	max = triangles[0]->points[0];

	for (Triangle * triangle : triangles) {
		for (glm::vec3 point : triangle->points) {
			for (int i = 0; i < 3; i++) {
				if (point[i] < min[i]) {
					min[i] = point[i];
				}
				else if (point[i] > max[i]) {
					max[i] = point[i];
				}
			}		
		}
		
	}
}


bool Mesh :: isHit (glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {
	
	for (Triangle * triangle : triangles) {
		if (triangle->isHit(rayOrigin, rayDir, minRayLen, maxRayLen, inside)) {
			material = triangle->material;
			normal = triangle->normal;
			rayLen = triangle->rayLen;
			
			// This is incorrect, need to hit the closest triangle!
			return true;
		}
	}

	return false;
}

void Mesh::translate(glm::vec3 vector) {
	for (Triangle * triangle : triangles) {
		for(int i=0; i<3; i++){
			triangle->points[i] += vector;
		}
	}
}

void Mesh::resetOrigin() {
	findSlabs();
	origin = (min + max) / 2.0f;
}

void Mesh::scale(float scale) {		
	for (Triangle* triangle : triangles) {
		for (int i = 0; i < 3; i++) {
			glm::vec3 currPos = triangle->points[i] - origin;
			glm::vec3 newPos = currPos * scale;
			triangle->points[i]+= newPos-currPos;
		}
	}	
}


void Mesh::addQuaternion(glm::vec3 axis, float angle) {
	q = glm::rotate(q, angle, normalize(axis));
}

void Mesh::rotate() {		
	for (Triangle* triangle : triangles) {
		for (int i = 0; i < 3; i++) {
			glm::vec3 currPos = triangle->points[i] - origin;
			glm::vec3 newPos = currPos * mat3_cast(q);
			triangle->points[i] += newPos - currPos;
		}
	}	
}


void Mesh::debug() {
	printf("Mesh @ RayLen = %f\n", rayLen);
}


/*************************************************************************/
/* Acceleration */


bool BoundingVolume::build(Mesh* newMesh, int threshold, int maxDepth, int currDepth) {

	mesh = newMesh;
	mesh->resetOrigin();

	// Mesh contains a minimum number of objects, this is a base case.
	if (mesh->triangles.size() <= threshold || currDepth >= maxDepth)	{
		//printf("Leave node has this number of triangles = %d\n", mesh->triangles.size());
		isLeave = true;
		return true;
	}
	   
	// Else, classify each triangle to 1 of the 8 nodes
	int nodePointsNum[8] = { 0 };

	for (Triangle* triangle : mesh->triangles) {

		glm::vec3 triCenter = triangle->getBarycenter();
	
		//printf("Barycenter = %f, %f, %f", triCenter.x, triCenter.y, triCenter.z);

		triangle->nodeID = 0;
		if (triCenter.x > mesh->origin.x) {
			triangle->nodeID += 1;
		}
		if (triCenter.y > mesh->origin.y) {
			triangle->nodeID += 2;
		}
		if (triCenter.z > mesh->origin.z) {
			triangle->nodeID += 4;
		}

		// Remember which nodes have triangles. This allows to ignore empty nodes
		nodePointsNum[triangle->nodeID]++;
	}
	   
	for (int i = 0; i < 8; i++)	{
		if (nodePointsNum[i]!=0) {
			//printf("Node %d, number of triangles = %d\n", i, nodePointsNum[i]);

			children[i] = new BoundingVolume();

			std::vector<Triangle*> subset;

			for (Triangle* triangle : mesh->triangles) {
				if (triangle->nodeID == i) {
					subset.push_back(triangle);
				}
			}

			Mesh* subMesh = new Mesh();
			subMesh->triangles = subset;
			children[i]->build(subMesh, threshold, maxDepth, currDepth + 1);	
		}
	}
	return true;
}


bool BoundingVolume::isHit(glm::vec3 rayOrigin, glm::vec3 rayDir, float minRayLen, float maxRayLen, bool inside) {
	
	// Optimization for AABB intersection test
	// https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525

	glm::vec3 invRayDir = 1.0f / rayDir;
	glm::vec3 t0 = (mesh->min - rayOrigin) * invRayDir;
	glm::vec3 t1 = (mesh->max - rayOrigin) * invRayDir;

	glm::vec3 tmin = glm::min(t0, t1);
	glm::vec3 tmax = glm::max(t0, t1);

	float lowest = glm::max(minRayLen, glm::max(tmin[0], glm::max(tmin[1], tmin[2])));
	float highest = glm::min(maxRayLen, glm::min(tmax[0], glm::min(tmax[1], tmax[2])));

	if (lowest <= highest) { // Bounding Volume was hit, checking children
		
		if (isLeave) { // Checking each triangle on the mesh			
			if (mesh->isHit(rayOrigin, rayDir, minRayLen, maxRayLen, inside)) {
				rayLen = mesh->rayLen;
				normal = mesh->normal;
				material = mesh->material;
				return true;
			}
			else {
				return false;
			}
		}
		else { // Checking the next round of Bounding Volumes	
			rayLen = maxRayLen;

			for (BoundingVolume* child : children) {
				if (child) { 
					if (child->isHit(rayOrigin, rayDir, minRayLen, maxRayLen, inside)) {
						if (child->rayLen < rayLen) { // looking for the closest hit
							rayLen = child->rayLen;
							normal = child->normal;
							material = child->material;
						}						
						return true;
					}
				}
			}
		}			
	}
	return false;
}

void BoundingVolume::debug() {
	printf("BoundingVolume @ RayLen = %f\n", rayLen);
}
