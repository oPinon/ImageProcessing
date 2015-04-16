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
	Image src = loadImage(srcFileName);
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

	Image low = resize(src, src.width / step, src.height / step, down);
	Image srcB = resize(low, src.width, src.height, up);

	free(low.img);

	Image dst = copy(src);

	do {
		Image dstB = resize(dst, dst.width*step, dst.height*step, up);
		free(dst.img);
		dst = superSample(src, srcB, dstB);
		free(dstB.img);
		//cout << " [ " << dst.width << "; " << dst.height << " ]" << endl;
		
	} while (dst.width < width || dst.height < height);

	writeImage(dst, dstFileName);

	free(srcB.img);
	free(dst.img);
	free(src.img);

	return 0;
}