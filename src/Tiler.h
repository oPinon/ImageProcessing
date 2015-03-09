#pragma once

#include "Image.h"
#include <vector>
#include <direct.h>

// Takes one very big image and makes it several tiles
// to be used for a map explorer such as Leaflet
// the tiles' folder structure is folder/{z}/{x}/{y}.png
// where int z = zoom (starts a 0 with one tile)
// int x and y are the tile positions

typedef unsigned int uint;

void image2Tiles(const Image& img, uint tileSize, std::string mainFolder);

// reduces the size of an image by 2
Image downscaleBy2(const Image& img);
