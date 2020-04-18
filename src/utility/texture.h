#ifndef texture_h // include guard
#define texture_h

#include "EasyBMP/EasyBMP.h"	
#include <glm/glm.hpp>  // glm

enum class TextureMode { none, checkers, custom };

class Texture {
public:
	TextureMode mode = TextureMode::none;
	BMP data;
	float scaleU;
	float scaleV;

	void loadBMP(const char* location);
	glm::vec3 getPixel(float u, float v);
};


#endif texture_h