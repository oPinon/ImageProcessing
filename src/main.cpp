// standard libraries
#include <iostream>
#include <time.h>

// external libraries
#include "lodepng.h"

// project libraries
#include "Image.h"
#include "Blur.h"
#include "Upscaling.h"
#include "VoxelTexture.h"

using namespace std;

int main(int argc, char** argv) {

	makeVoxelTexture(argc, argv);

	return 0;
}