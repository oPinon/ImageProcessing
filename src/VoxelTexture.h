#pragma once

#include <vector>
#include <string>
#include "Image.h"
#include "lodepng.h"
#include <iostream>

Image loadVoxelImages(std::string folder, int digits_nb);

void makeVoxelTexture(int argc, char** argv);