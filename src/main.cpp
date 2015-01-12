// standard libraries
#include <iostream>
#include <time.h>

// external libraries
#include "lodepng.h"

// project libraries
#include "Image.h"
#include "Filtering.h"
#include "Upscaling.h"
#include "VoxelTexture.h"

using namespace std;

int main(int argc, char** argv) {

	//makeVoxelTexture(argc, argv);

	Image src = loadImage("../images/LicheusHigh.png");

	Image result = blur(src, 64);

	writeImage(result,"../images/Result.png");

	return 0;
}