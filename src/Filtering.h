#pragma once

#include <vector>
#include "Image.h"

Image processKernel(const Image& src, unsigned int kernelSize, void enter(unsigned char* pixel), void exit(unsigned char* pixel), void set(unsigned char* srcPixel, unsigned char* dstPixel));

Image blur(const Image& src, int k);

Image adaptiveHistogramEqualization(const Image& src, int k);

Image expand(const Image& src, int border);

Image crop(const Image& src, int border);

template<typename T>
T clamp(const T v, const T minV, const T maxV) {
	
	if (v < minV) { return minV; }
	if (v > maxV) { return maxV; }
	return v;
}