#include "Upscaling.h"

Image scaleNearest(Image& img, float scale) {

	unsigned int src_width = img.width, src_height = img.height;
	unsigned int dst_width = img.width * scale, dst_height = img.height * scale;

	Image toReturn = { std::vector<unsigned char>(dst_width*dst_height*4), dst_width, dst_height };

	unsigned char* src = &img.img[0];
	unsigned char* dst = &toReturn.img[0];

	for (unsigned int color = 0; color < 4; color++) {
		for (unsigned int x = 0; x < dst_width; x++) {
			for (unsigned int y = 0; y < dst_height; y++) {

				unsigned int x0 = x / scale;
				unsigned int y0 = y / scale;
				dst[4 * (x + y*dst_width) + color] = src[4 * (x0 + y0*src_width) + color];
			}
		}
	}

	return toReturn;
}