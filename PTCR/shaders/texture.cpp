#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include "texture.h"

bool texture::load(const string filename) {
	int n = 4;
	int width = 0, height = 0;
	uchar* tmp = stbi_load(filename.c_str(), &width, &height, &n, 4);
	if (tmp != nullptr) {
		data = tmp;
		w = width;
		h = height;
		return true;
	}
	return false;
}

