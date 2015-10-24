#include "Filtering.h"

typedef unsigned int uint;
typedef unsigned char uchar;

uint counterR, counterG, counterB, counterA;

void enterBlur(uchar* pixel) {
	counterR += pixel[0];
	counterG += pixel[1];
	counterB += pixel[2];
	counterA += pixel[3];
}
void exitBlur(uchar* pixel) {
	counterR -= pixel[0];
	counterG -= pixel[1];
	counterB -= pixel[2];
	counterA -= pixel[3];
}
uint totalBlur = 1;
void setBlur(uchar* srcPixel, uchar* dstPixel) {
	dstPixel[0] = counterR / totalBlur;
	dstPixel[1] = counterG / totalBlur;
	dstPixel[2] = counterB / totalBlur;
	dstPixel[3] = counterA / totalBlur;
}

Image blur(const Image& src, int k) {

	counterR = 0; counterG = 0; counterB = 0; counterA = 0;
	totalBlur = (2 * k + 1)*(2 * k + 1);
	return processKernel(src, k, enterBlur, exitBlur, setBlur);
}

Image processKernel(const Image& srcI0, uint kernelSize, void enter(uchar* pixel), void exit(uchar* pixel), void set(uchar* srcPixel, uchar* dstPixel)) {

	Image srcI = expand(srcI0, kernelSize);
	int width = srcI.width, height = srcI.height;
	Image dstI = newImage(width, height);

	uchar* src = &srcI.img[0];
	uchar* dst = &dstI.img[0];

	// Initialize Kernel
	for (uint x = 0; x < 2 * kernelSize + 1; x++) {
		for (uint y = 0; y < 2 * kernelSize + 1; y++) {
			enter( src + (4 * (x + width*y)) );
		}
	}

	bool forward = true;
	uint counter = 0, end = (width - 2 * kernelSize)*(height - 2 * kernelSize);
	uint x0 = kernelSize, y0 = kernelSize;

	// Process all pixels
	while (true) {

		set( src + (4 * (x0 + width*y0)), dst + (4 * (x0 + width*y0)));

		if (counter == end - 1) { break; }
		else { counter++; }

		// if on left or right side, go y++
		if ((forward && (x0 == width - 1 - kernelSize)) || (!forward && (x0 == kernelSize))) {

			for (uint x = x0 - kernelSize; x <= x0 + kernelSize; x++) {

				exit( src + (4 * (x + width* (y0 - kernelSize))) );
				enter( src + (4 * (x + width* (y0 + kernelSize + 1))) );
			}
			y0++;
			forward = !forward;
		}
		else { // else go left or right

			int step = forward ? 1 : -1;

			for (uint y = y0 - kernelSize; y <= y0 + kernelSize; y++) {

				exit( src + (4 * ((x0 - step*kernelSize) + width* y)) );
				enter( src + (4 * ((x0 + step*(kernelSize + 1)) + width* y)) );
			}
			x0 += step;
		}
	}

	return crop(dstI,kernelSize);
}

Image expand(const Image& src, int border) {

	Image dst = newImage(src.width + 2 * border, src.height + 2 * border);
	for (int y = 0; y < dst.height; y++) {
		int y2 = clamp<int>(y - border, 0, src.height-1);
		for (int x = 0; x < dst.width; x++) {
			int x2 = clamp<int>(x - border, 0, src.width-1);
			for (int k = 0; k < 4; k++) {
				dst.img[4 * (dst.width*y + x) + k] = src.img[4 * (src.width*y2 + x2) + k];
			}
		}
	}
	return dst;
}

Image crop(const Image& src, int border) {

	if (src.width <= 2 * border || src.height <= 2 * border) { throw 1; }
	Image dst = newImage(src.width - 2 * border, src.height - 2 * border);
	for (int y = 0; y < dst.height; y++) {
		int y2 = clamp<int>(y + border, 0, src.height-1);
		for (int x = 0; x < dst.width; x++) {
			int x2 = clamp<int>(x + border, 0, src.width-1);
			for (int k = 0; k < 4; k++) {
				dst.img[4 * (dst.width*y + x) + k] = src.img[4 * (src.width*y2 + x2) + k];
			}
		}
	}
	return dst;
}

// TODO : don't use these variables if the algorithm isn't used
// solution : use objects

uchar histogramR[256];
uchar histogramG[256];
uchar histogramB[256];
uint sum = 0;

void enterAHE(uchar* pixel) {

	histogramR[pixel[0]]++;
	histogramG[pixel[1]]++;
	histogramB[pixel[2]]++;
	sum++;
}

void exitAHE(uchar* pixel) {

	histogramR[pixel[0]]--;
	histogramG[pixel[1]]--;
	histogramB[pixel[2]]--;
	sum--;
}

void setAHE(uchar* srcPixel, uchar* dstPixel) {

	uint count = 0;
	for (uchar i = 0; i < srcPixel[0]; i++) {
		count += histogramR[i];
	}
	dstPixel[0] = clamp<uchar>((count * 255) / sum,0,255);
	count = 0;
	for (uchar i = 0; i < srcPixel[1]; i++) {
		count += histogramG[i];
	}
	dstPixel[1] = clamp<uchar>((count * 255) / sum, 0, 255);
	count = 0;
	for (uchar i = 0; i < srcPixel[2]; i++) {
		count += histogramB[i];
	}
	dstPixel[2] = clamp<uchar>((count * 255) / sum, 0, 255);
	dstPixel[3] = 255;
}

Image adaptiveHistogramEqualization(const Image& src, int k) {

	for (uint i = 0; i < 256; i++) {
		histogramR[i] = 0;
		histogramG[i] = 0;
		histogramB[i] = 0;
	}
	//sum = k*k;
	return processKernel(src, k, enterAHE, exitAHE, setAHE);
}