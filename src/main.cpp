// standard libraries
#include <iostream>
#include <time.h>
#include <sstream>

// project libraries
#include "SuperSample.h"

using namespace std;

float Max(const float a, const float b) { if (a > b) { return a; } else { return b; } }

float error(const Image& im1, const Image& im2) {

	if (im1.channels != im2.channels || im1.width != im2.width || im1.height != im2.height) { cerr << "comparing images with different dimensions" << endl; return -1; }
	float sum = 0;
	for (uint i = 0; i < im1.channels*im1.width*im1.height; i++) {
		float pix1 = im1.img[i]; float pix2 = im2.img[i];
		if (pix1 != 0 || pix2 != 0) {
			sum += abs( pix1 - pix2 ) / Max( pix1, pix2 );
		}
	}
	return sum / im1.channels*im1.width*im1.height;
}

float step = 2;
uint i = 0;

Image Upscale(const Image& src, Interpolation down, Interpolation up) {

	Image srcB = resize( resize(src, src.width / step, src.height / step, down), src.width, src.height, up);
	Image dstB = resize(src, src.width * step, src.height * step, up);
	Image temp = superSample(src, srcB, dstB);
	dstB = resize(temp, temp.width*step, temp.height*step, up);
	Image dst = superSample(src, srcB, dstB);

	ostringstream oss; oss << "../images/" << i << ".png"; i++;
	writeImage(dst, oss.str().c_str() );
	return dst;
}

int main(int argc, char** argv) {

	Image src = loadImage("../images/gt.png");

	/*Image low = resize(src, src.width / 4, src.height / 4, LINEAR);
	writeImage(low, "../images/low.png");*/

	Image low = loadImage("../images/src.png");

	cout << "errors for each upscaling methods (the lowest the better) :" << endl;
	cout << "Nearest Neighbor : " << error(src, resize(low, src.width, src.height, NEAREST)) << endl;
	cout << "Linear : " << error(src, resize(low, src.width, src.height, LINEAR)) << endl;
	cout << "Sinc : " << error(src, resize(low, src.width, src.height, SINC)) << endl;
	cout << "Cubic10 : " << error(src, resize(low, src.width, src.height, CUBIC10)) << endl;

	cout << "Sup : down = NN, up = CubicC10 : " << error(src, Upscale( low, NEAREST, CUBIC10)) << endl;
	cout << "Sup : down = Linear, up = CubicC10 : " << error(src, Upscale(low, LINEAR, CUBIC10)) << endl;
	cout << "Sup : down = Cubic10, up = CubicC10 : " << error(src, Upscale(low, CUBIC10, CUBIC10)) << endl;

	cout << "Sup : down = NN, up = Linear : " << error(src, Upscale(low, NEAREST, LINEAR)) << endl;
	cout << "Sup : down = Linear, up = Linear : " << error(src, Upscale(low, LINEAR, LINEAR)) << endl;
	cout << "Sup : down = Cubic10, up = Linear : " << error(src, Upscale(low, CUBIC10, LINEAR)) << endl;

	return 0;
}