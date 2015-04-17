// standard libraries
#include <iostream>
#include <time.h>
#include <string>

// project libraries
#include "SuperSample.h"

using namespace std;

int main(int argc, char** argv) {

	if (argc < 3) {
		cout << "command line arguments are : <input_image.png> <output_image.png> (<width> <height> optional)" << endl;
	}

	char* srcFileName = argv[1];
	char* dstFileName = argv[2];
	Image srcU = loadImage(srcFileName);
	ImageF src = convert(srcU);
	free(srcU.img);
	uint width, height;

	if (argc == 3) {
		width = 4 * src.width;
		height = 4 * src.height;
	}
	else {
		width = stoi(argv[3]);
		if (argc == 4) {
			height = (width * src.height) / src.width;
		}
		else {
			height = stoi(argv[4]);
		}
	}

	float step = 1.25;
	Interpolation up = LANCZOS;
	Interpolation down = up;

	ImageF low = resize(src, src.width / step, src.height / step, down);
	ImageF srcB = resize(low, src.width, src.height, up);

	free(low.img);

	ImageF dst = copy(src);

	do {
		ImageF dstB = resize(dst, dst.width*step, dst.height*step, up);
		free(dst.img);
		dst = superSample(src, srcB, dstB);
		free(dstB.img);
		//cout << " [ " << dst.width << "; " << dst.height << " ]" << endl;
		
	} while (dst.width < width || dst.height < height);

	Image dstU = convert(dst);
	writeImage(dstU, dstFileName);

	free(srcB.img);
	free(dst.img);
	free(src.img);
	free(dstU.img);

	return 0;
}