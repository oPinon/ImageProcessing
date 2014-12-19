#include "Blur.h"
#include <vector>

Image blur(Image& img, unsigned int kernelSize) {

	unsigned int width = img.width, height = img.height;
	unsigned char* src = &img.img[0];

	Image tempImage(img);
	unsigned char* tmp = &tempImage.img[0];
	Image toReturn(img);
	unsigned char* dst = &toReturn.img[0];

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

	tmp = &tempImage.img[0];

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

	return toReturn;

}