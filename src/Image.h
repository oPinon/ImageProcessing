#pragma once

#include <vector>

struct Image {
	// RGBA RGBA RGBA. y*width + x
	std::vector<unsigned char> img;
	unsigned int width, height;
};