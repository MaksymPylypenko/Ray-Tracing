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
	plane->position = points[0];

	glm::vec3 N = normalize(cross((points[1] - points[0]), (points[2] - points[0])));
	inside == true ? plane->normal = -N : plane->normal = N;
	   

	if (plane->isHit(rayOrigin, rayDir, minRayLen, maxRayLen, inside)) {
		glm::vec3 hitPos = rayOrigin + plane->rayLen * rayDir;

		bool axProj = dot(cross((points[1] - points[0]), (hitPos - points[0])), plane->normal) < 0;
		bool bxProj = dot(cross((points[2] - points[1]), (hitPos - points[1])), plane->normal) < 0;
		bool cxProj = dot(cross((points[0] - points[2]), (hitPos - points[2])), plane->normal) < 0;
		
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

void Mesh::scale(float scale, bool findOrigin) {

	if (findOrigin) { // Find a current barycenter and make it an origin
		findSlabs();
		glm::vec3 origin = (min + max) / 2.0f;
		for (Triangle* triangle : triangles) {
			for (int i = 0; i < 3; i++) {
				glm::vec3 currPos = triangle->points[i] - origin;
				glm::vec3 newPos = currPos * scale;
				triangle->points[i]+= newPos-currPos;
			}
		}
	}
	else { // Assume a correct origin
		for (Triangle* triangle : triangles) {
			for (int i = 0; i < 3; i++) {
				glm::vec3 currPos = triangle->points[i];
				glm::vec3 newPos = triangle->points[i] *= scale;
				triangle->points[i] += newPos - currPos;
			}
		}
	}	
}


void Mesh::rotate(glm::vec3 axis, float angle, bool findOrigin) {	

	// Quaternion math is taken from this tutorial 
	// https://www.geeks3d.com/20141201/how-to-rotate-a-vertex-by-a-quaternion-in-glsl/


	// Finding quaternion component x,y,z,w from [axis] + [angle] 
	float half_angle = (angle * 0.5) * 3.14159 / 180.0;
	glm::vec3 xyz = axis * sin(half_angle);	
	float w = cos(half_angle);

	if (findOrigin) { // Find a current barycenter and make it an origin
		findSlabs();
		glm::vec3 origin = (min + max) / 2.0f;
		for (Triangle* triangle : triangles) {
			for (int i = 0; i < 3; i++) {
				glm::vec3 currPos = triangle->points[i] - origin;
				glm::vec3 newPos = currPos + 2.0f * cross(xyz, cross(xyz, currPos) + w * currPos);
				triangle->points[i] += newPos - currPos;
			}
		}
	}
	else {
		for (Triangle* triangle : triangles) {
			for (int i = 0; i < 3; i++) {
				glm::vec3 currPos = triangle->points[i];
				glm::vec3 newPos = currPos + 2.0f * cross(xyz, cross(xyz, currPos) + w * currPos);
				triangle->points[i] += newPos - currPos;
			}
		}
	}
	
}



void Mesh::debug() {
	printf("Mesh @ RayLen = %f\n", rayLen);
}


/*************************************************************************/
/* Acceleration */


bool BoundingVolume::build(Mesh* mesh, int threshold, int maxDepth, int currDepth) {

	this->mesh = mesh;

	// Mesh contains a minimum number of objects, this is a base case.
	if (mesh->triangles.size() <= threshold || currDepth >= maxDepth)	{
		//printf("Leave node has this number of triangles = %d\n", mesh->triangles.size());
		isLeave = true;
		return true;
	}
	   
	// Else, classify each triangle to 1 of the 8 nodes
	int nodePointsNum[8] = { 0 };
	glm::vec3 meshCenter = (mesh->max + mesh->min) * 0.5f;  

	for (Triangle* triangle : mesh->triangles) {

		glm::vec3 triCenter = triangle->getBarycenter();
	
		//printf("Barycenter = %f, %f, %f", triCenter.x, triCenter.y, triCenter.z);

		triangle->nodeID = 0;
		if (triCenter.x > meshCenter.x) {
			triangle->nodeID += 1;
		}
		if (triCenter.y > meshCenter.y) {
			triangle->nodeID += 2;
		}
		if (triCenter.z > meshCenter.z) {
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
			subMesh->findSlabs();

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
