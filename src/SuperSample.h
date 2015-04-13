#pragma once

#include <iostream>
using namespace std;

typedef unsigned char uchar;
typedef unsigned int uint;

typedef struct _Image {
	uchar* img;
	uint width, height, channels;
} Image;

typedef struct _ImageD {
	int* img; // sum of values of each pixels
	uint* count; // nb of values for each pixels (for future averaging)
	uint width, height, channels;
} ImageD;

typedef enum { NEAREST, LINEAR, SINC, CUBIC10 } Interpolation;

Image loadImage(const char* filename);
void writeImage(Image im, const char* filename);

/*
* src : source image
* srcB : blurred version of src
* dstB : blurred version of the destination image
*/
Image superSample(Image src, Image srcB, Image dstB);

Image resize(const Image& src, const uint dstWidth, const uint dstHeight, Interpolation interpolation);

Image down_5_4(const Image& src);

Image up_5_4(const Image& src);
