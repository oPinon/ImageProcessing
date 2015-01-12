#include "Filtering.h"

unsigned int counterR, counterG, counterB, counterA;

void enterBlur(unsigned char* pixel) {
	counterR += pixel[0];
	counterG += pixel[1];
	counterB += pixel[2];
	counterA += pixel[3];
}
void exitBlur(unsigned char* pixel) {
	counterR -= pixel[0];
	counterG -= pixel[1];
	counterB -= pixel[2];
	counterA -= pixel[3];
}
unsigned int totalBlur = 1;
void setBlur(unsigned char* pixel) {
	pixel[0] = counterR / totalBlur;
	pixel[1] = counterG / totalBlur;
	pixel[2] = counterB / totalBlur;
	pixel[3] = counterA / totalBlur;
}

Image blur(Image& src, int k) {

	counterR = 0; counterG = 0; counterB = 0; counterA = 0;
	totalBlur = (2 * k + 1)*(2 * k + 1);
	return processKernel(src, k, enterBlur, exitBlur, setBlur);
}

Image processKernel(Image& srcI, unsigned int kernelSize, void enter(unsigned char* pixel), void exit(unsigned char* pixel), void set(unsigned char* pixel)) {

	int width = srcI.width, height = srcI.height;
	Image dstI = newImage(width, height);

	unsigned char* src = &srcI.img[0];
	unsigned char* dst = &dstI.img[0];

	// Initialize Kernel
	for (unsigned int x = 0; x < 2 * kernelSize + 1; x++) {
		for (unsigned int y = 0; y < 2 * kernelSize + 1; y++) {
			enter( src + (4 * (x + width*y)) );
		}
	}

	bool forward = true;
	unsigned int counter = 0, end = (width - 2 * kernelSize)*(height - 2 * kernelSize);
	unsigned int x0 = kernelSize, y0 = kernelSize;

	// Process all pixels
	while (true) {

		set( dst + (4 * (x0 + width*y0)) );

		if (counter == end - 1) { break; }
		else { counter++; }

		// if on left or right side, go y++
		if ((forward && (x0 == width - 1 - kernelSize)) || (!forward && (x0 == kernelSize))) {

			for (unsigned int x = x0 - kernelSize; x <= x0 + kernelSize; x++) {

				exit( src + (4 * (x + width* (y0 - kernelSize))) );
				enter( src + (4 * (x + width* (y0 + kernelSize + 1))) );
			}
			y0++;
			forward = !forward;
		}
		else { // else go left or right

			int step = forward ? 1 : -1;

			for (unsigned int y = y0 - kernelSize; y <= y0 + kernelSize; y++) {

				exit( src + (4 * ((x0 - step*kernelSize) + width* y)) );
				enter( src + (4 * ((x0 + step*(kernelSize + 1)) + width* y)) );
			}
			x0 += step;
		}
	}

	return dstI;
}