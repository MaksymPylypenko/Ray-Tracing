// The JSON library allows you to reference JSON arrays like C++ vectors and JSON objects like C++ maps.

#include "scene_adapter.h"
#include "texture.h"

const char* PATH = "scenes/";



json find(json& j, const std::string key, const std::string value) {
	// Utility function
	json::iterator it;
	for (it = j.begin(); it != j.end(); ++it) {
		if (it->find(key) != it->end()) {
			if ((*it)[key] == value) {
				return *it;
			}
		}
	}
	return json();
}

glm::vec3 vector_to_vec3(const std::vector<float>& v) {
	return glm::vec3(v[0], v[1], v[2]);
}


void SceneAdapter::choose_scene(char const* fn) {
	if (fn == NULL) {
		std::cout << "Using default input file " << PATH << "c.json\n";
		fn = "c";
	}

	std::cout << "Loading scene " << fn << std::endl;

	std::string fname = PATH + std::string(fn) + ".json";
	std::fstream in(fname);
	if (!in.is_open()) {
		std::cout << "Unable to open scene file " << fname << std::endl;
		exit(EXIT_FAILURE);
	}

	in >> scene;

	json camera = scene["camera"];
	// these are optional parameters (otherwise they default to the values initialized earlier)
	if (camera.find("field") != camera.end()) {
		fov = camera["field"];
		std::cout << "Setting fov to " << fov << " degrees.\n";
	}
	if (camera.find("background") != camera.end()) {
		background_colour = vector_to_vec3(camera["background"]);
		std::cout << "Setting background colour to " << glm::to_string(background_colour) << std::endl;
	}
}


/****************************************************************************/

void SceneAdapter::jsonImport() {
	json& jsonObjects = scene["objects"];



	// traverse the objects	
	for (json::iterator it = jsonObjects.begin(); it != jsonObjects.end(); ++it) {
		json& object = *it;

		// Boolean operation (e.g. allows to make union or intersection of 2 objects)
		bool isNegative = false;

		if (object.find("subtract") != object.end()) {
			isNegative = object["subtract"];
		}

		// Textures	

		Texture* texture = new Texture();

		if (object.find("texture") != object.end()) {
			json& jsonTexture = object["texture"];

			texture->scaleU = jsonTexture["scale"];
			texture->scaleV = jsonTexture["scale"];

			if (jsonTexture.find("procedural") != jsonTexture.end()) {
				json& procedural = jsonTexture["procedural"];
				if(procedural == "checkers") {
					texture->mode = TextureMode::checkers;
				}
			}

			if (jsonTexture.find("custom") != jsonTexture.end()) {
				texture->mode = TextureMode::custom;
				json& custom = jsonTexture["custom"];
				if (custom == "lava") {
					texture->loadBMP("textures/lava24.bmp");
				}
				else if (custom == "stone") {
					texture->loadBMP("textures/stone24.bmp");
				}
				else if (custom == "paper") {
					texture->loadBMP("textures/paper24.bmp");
				}
				else if (custom == "metal") {
					texture->loadBMP("textures/metal24.bmp");
				}
				else if (custom == "obsidian") {
					texture->loadBMP("textures/obsidian24.bmp");
				}
			}			
		}
		else {
			texture->mode = TextureMode::none;
		}

	
		// Materials	

		Material* material = new Material();
		json& jsonMaterial = object["material"];

		if (jsonMaterial.find("ambient") != jsonMaterial.end()) {
			material->Ka = vector_to_vec3(jsonMaterial["ambient"]);
		}
		if (jsonMaterial.find("diffuse") != jsonMaterial.end()) {
			material->Kd = vector_to_vec3(jsonMaterial["diffuse"]);
		}
		if (jsonMaterial.find("specular") != jsonMaterial.end()) {
			material->Ks = vector_to_vec3(jsonMaterial["specular"]);
		}
		if (jsonMaterial.find("shininess") != jsonMaterial.end()) {
			material->shininess = jsonMaterial["shininess"];
		}
		else {
			material->shininess = 0.0;
		}
		if (jsonMaterial.find("reflective") != jsonMaterial.end()) {
			material->reflection = vector_to_vec3(jsonMaterial["reflective"]);
		}
		if (jsonMaterial.find("transmissive") != jsonMaterial.end()) {
			material->transmission = vector_to_vec3(jsonMaterial["transmissive"]);
		}
		if (jsonMaterial.find("refraction") != jsonMaterial.end()) {
			material->refraction = jsonMaterial["refraction"];
		}
		else {
			material->refraction = 0.0;
		}
		

		// Objects

		if (object["type"] == "sphere") {
			Sphere* sphere = new Sphere();
			sphere->radius = float(object["radius"]);
			sphere->center = vector_to_vec3(object["position"]);
			sphere->material = material;	
			if (texture->mode != TextureMode::none) {
				texture->scaleU *= 2; // adjusting for polar coordinates
			}
			sphere->texture = texture;
			sphere->isNegative = isNegative;
			objects.push_back(sphere);
		}

		else if (object["type"] == "plane") {
			Plane* plane = new Plane();
			plane->center = vector_to_vec3(object["position"]);
			plane->normal = normalize(vector_to_vec3(object["normal"]));
			plane->material = material;		
			plane->alignTextureAxes();
			plane->texture = texture;
			plane->isNegative = isNegative;
			objects.push_back(plane);
		}

		else if (object["type"] == "mesh") {

			std::vector<Triangle*> triangles;
						
			for (std::vector<std::vector<float>> triangleJson : object["triangles"]) {
				Triangle* triangle = new Triangle();
				triangle->points.push_back(vector_to_vec3(triangleJson[0]));
				triangle->points.push_back(vector_to_vec3(triangleJson[1]));
				triangle->points.push_back(vector_to_vec3(triangleJson[2]));
				triangle->material = material;				
				triangle->texture = texture;		
				triangle->isNegative = isNegative;
				triangles.push_back(triangle);
			}
						
			Mesh* mesh = new Mesh();
			mesh->triangles = triangles;

			if (object.find("transform") != object.end()) {
				json& jsonTransform = object["transform"];
				if (jsonTransform.find("scale") != jsonTransform.end()) {
					mesh->scale(jsonTransform["scale"]);
				}
				if (jsonTransform.find("translate") != jsonTransform.end()) {
					mesh->translate(vector_to_vec3(jsonTransform["translate"]));
				}			
				
				if (jsonTransform.find("new_origin") != jsonTransform.end()) {
					if ((jsonTransform["new_origin"]) == true) {
						mesh->resetOrigin();
					}
				}
				if (jsonTransform.find("scale") != jsonTransform.end()) {
					mesh->scale(jsonTransform["scale"]);					
				}
				if (jsonTransform.find("rotation") != jsonTransform.end()) {
					json& jsonRot = jsonTransform["rotation"];
					glm::vec3 axis = vector_to_vec3(jsonRot["axis"]);
					float angle = jsonRot["angle"];
					mesh->addQuaternion(axis, angle);
					mesh->rotate();
				}					
			}
						
			mesh->resetOrigin();
			mesh->resetBarycenters();

			MeshHierarchy* mh = new MeshHierarchy();	
			mh->build(mesh);			

			objects.push_back(mh);
		}
	}	

	json& jsonLights = scene["lights"];

	// traverse the light sources 
	for (json::iterator it = jsonLights.begin(); it != jsonLights.end(); ++it) {
		json& light = *it;

		if (light["type"] == "ambient") {
			Ambient* ambient = new Ambient();
			ambient->colour = vector_to_vec3(light["color"]);
			lights.push_back(ambient);
		}
		else if (light["type"] == "point") {
			Point* point = new Point();
			point->colour = vector_to_vec3(light["color"]);
			point->position = vector_to_vec3(light["position"]);
			lights.push_back(point);
		}
		else if (light["type"] == "directional") {
			Directional* directional = new Directional();
			directional->colour = vector_to_vec3(light["color"]);
			directional->direction = normalize(vector_to_vec3(light["direction"]));
			lights.push_back(directional);
		}
		else if (light["type"] == "spot") {
			Spot* spot = new Spot();
			spot->colour = vector_to_vec3(light["color"]);
			spot->direction = normalize(vector_to_vec3(light["direction"])); // oh well..
			spot->position = vector_to_vec3(light["position"]);
			spot->cutoff = float(light["cutoff"]);
			lights.push_back(spot);
		}
	}

	//Area* area = new Area();
	//area->position = glm::vec3(1, 3, 0);
	//area->dirU = glm::vec3(0, 0, 1);
	//area->dirV = glm::vec3(-1, 0, 0);
	//area->distU = 2.0f;
	//area->distV = 2.0f;
	//area->normal = normalize(cross(area->dirU, area->dirV));
	//area->colour = glm::vec3(0.8, 0.8, 0.8);
	//lights.push_back(area);

}
