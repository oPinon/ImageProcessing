#pragma once

#include <vector>
#include <string>
#include "Image.h"
#include "lodepng.h"
#include <iostream>

/*
*	Takes a sequence of images and puts then in one big image grid
*/

Image loadVoxelImages(std::string folder, int digits_nb);

void makeVoxelTexture(int argc, char** argv);