// The JSON library allows you to reference JSON arrays like C++ vectors and JSON objects like C++ maps.

#include "texture.h"


glm::vec3 Texture::getPixel(float u, float v) {

	if (mode == TextureMode::custom) {
		u = round(abs(u * scaleU));
		v = round(abs(v * scaleV));

		int x = int(u) % data.TellWidth();
		int y = int(v) % data.TellHeight();

		float r = data(x, y)->Red / 255.0f;
		float g = data(x, y)->Green / 255.0f;
		float b = data(x, y)->Blue / 255.0f;

		return glm::vec3(r, g, b);
	}

	else if (mode == TextureMode::checkers) {
		bool white = (int(round(u * scaleU)) + int(round(v * scaleV))) % 2 == 0;
		if (white) {
			return glm::vec3(1.0, 1.0, 1.0);
		}
		else {
			return glm::vec3(0.4, 0.4, 0.4);
		}
	}
	
}

void Texture::loadBMP(const char* location) {
	data.ReadFromFile(location);
}



