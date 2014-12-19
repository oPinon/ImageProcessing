#include "Blur.h"
#include <vector>

void blur(unsigned char* src, unsigned char* dst, unsigned int width, unsigned int height, unsigned int kernelSize) {

	std::vector<unsigned char> tempImage(width*height * 4);
	unsigned char* tmp = &tempImage[0];

	for (unsigned int line = 0; line < height; line++) {

		for (unsigned int color = 0; color < 4; color++) {

			// initialize kernel
			unsigned int sum = 0;
			for (unsigned int i = color; i < color + 2 * 4 * kernelSize; i += 4) {
				sum += src[i];
			}

			// Convolution
			for (unsigned int pixel = color + 4 * kernelSize; pixel < 4 * (width - kernelSize); pixel += 4) {
				sum -= src[pixel - 4 * kernelSize];
				sum += src[pixel + 4 * kernelSize];
				tmp[pixel] = sum / (2 * kernelSize);
			}
		}

		src += width * 4;
		tmp += width * 4;
	}

	tmp = &tempImage[0];

	for (unsigned int row = 0; row < width; row++) {

		for (unsigned int color = 0; color < 4; color++) {

			// initialize kernel
			unsigned int sum = 0;
			for (unsigned int i = 0; i < 2 * 4 * kernelSize; i += 4) {
				sum += tmp[i*width + color];
			}

			// Convolution
			for (unsigned int pixel = 4 * kernelSize; pixel < 4 * (height - kernelSize); pixel += 4) {
				sum -= tmp[(pixel - 4 * kernelSize)*width + color];
				sum += tmp[(pixel + 4 * kernelSize)*width + color];
				dst[pixel*width + color] = sum / (2 * kernelSize);
			}
		}

		tmp += 4;
		dst += 4;
	}

}