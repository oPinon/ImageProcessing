#pragma once

#include <iostream>
using namespace std;

typedef unsigned char uchar;
typedef unsigned int uint;

typedef struct _Image {
	uchar* img;
	uint width, height, channels;
} Image;

typedef struct _ImageF {
	float* img;
	uint width, height, channels;
} ImageF;

typedef struct _ImageD {
	double* img; // sum of values of each pixels
	uint* count; // nb of values for each pixels (for future averaging)
	uint width, height, channels;
} ImageD;

typedef enum { NEAREST, LINEAR, LANCZOS, CUBIC } Interpolation;

Image loadImage(const char* filename);
void writeImage(Image im, const char* filename); // TODO : currently works only for RGBA images

/*
* src : source image
* srcB : blurred version of src
* dstB : blurred version of the destination image
*/
ImageF superSample(ImageF src, ImageF srcB, ImageF dstB);

ImageF resize(const ImageF& src, const uint dstWidth, const uint dstHeight, Interpolation interpolation);

ImageF down_5_4(const ImageF& src);

ImageF up_5_4(const ImageF& src);

inline ImageF copy(const ImageF& src) {
	ImageF dst;
	dst.channels = src.channels;
	dst.width = src.width; dst.height = src.height;
	dst.img = (float*)malloc(src.channels*src.width*src.height*sizeof(float));
	for (uint i = 0; i < src.channels*src.width*src.height; i++) { dst.img[i] = src.img[i]; }
	return dst;
}

inline Image convert(const ImageF& src) {
	Image dst;
	dst.width = src.width; dst.height = src.height;
	dst.channels = src.channels;
	uint size = src.width*src.height*src.channels;
	dst.img = (uchar*)malloc(size*sizeof(uchar));
	for (uint i = 0; i < size; i++) {
		float v = src.img[i];
		if (v > 255) { v = 255; }
		else if (v < 0) { v = 0; }
		dst.img[i] = (uchar)(v+0.5);
	}
	return dst;
}

inline ImageF convert(const Image& src) {
	ImageF dst;
	dst.width = src.width; dst.height = src.height;
	dst.channels = src.channels;
	uint size = src.width*src.height*src.channels;
	dst.img = (float*)malloc(size*sizeof(float));
	for (uint i = 0; i < size; i++) {
		dst.img[i] = (float) src.img[i];
	}
	return dst;
}