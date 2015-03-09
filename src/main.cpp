// standard libraries
#include <iostream>
#include <time.h>

// project libraries
#include "Image.h"
#include "Filtering.h"
#include "Upscaling.h"
#include "VoxelTexture.h"
#include "Tiler.h"

using namespace std;

int main(int argc, char** argv) {

	//makeVoxelTexture(argc, argv);

	Image src = loadImage("../images/Big.png");

	image2Tiles(src, 256, "../images/Tiles");

	return 0;
}