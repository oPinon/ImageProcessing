#pragma once

#include <vector>
#include "lodepng.h"
#include <iostream>

struct Image {
	// RGBA RGBA RGBA. y*width + x
	std::vector<unsigned char> img;
	unsigned int width, height;
};

inline Image newImage(int width, int height) {
	Image toReturn = { std::vector<unsigned char>(width*height * 4), width, height };
	return toReturn;
}

inline Image loadImage(const std::string& filename) {
	Image toReturn;
	unsigned error = lodepng::decode(toReturn.img, toReturn.width, toReturn.height, filename);
	if (error) { std::cerr << "error " << error << " when opening " << filename << " : " << lodepng_error_text(error) << std::endl; }
	else { std::cout << "opened " << filename << " [ " << toReturn.width << " ; " << toReturn.height << " ] " << std::endl; }
	return toReturn;
}

inline void writeImage(const Image& img, const std::string& filename) {
	unsigned error = lodepng::encode(filename, img.img, img.width, img.height);
	if (error) { std::cerr << "error " << error << " when writing " << filename << " : " << lodepng_error_text(error) << std::endl; }
}