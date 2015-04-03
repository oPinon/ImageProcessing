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

Image loadImage(char* filename);
void writeImage(Image im, char* filename);

/*
* src : source image
* srcB : blurred version of src
* dstB : blurred version of the destination image
*/
Image superSample(Image src, Image srcB, Image dstB);

/*
* Resamples the source image's pixels to the destination image
* (using dst's width and height)
*/
void resize(const Image& src, Image& dst);

Image resize(const Image& src, const uint dstWidth, const uint dstHeight);