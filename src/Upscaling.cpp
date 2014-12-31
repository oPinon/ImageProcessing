#include "Upscaling.h"

Image scaleNearest(const Image& img, float scale) {

	unsigned int src_width = img.width, src_height = img.height;
	unsigned int dst_width = img.width * scale, dst_height = img.height * scale;

	Image toReturn = { std::vector<unsigned char>(dst_width*dst_height*4), dst_width, dst_height };

	const unsigned char* src = &img.img[0];
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

Image upscaleLinear(const Image& img, float scale) {

	unsigned int src_width = img.width, src_height = img.height;
	unsigned int dst_width = img.width * scale, dst_height = img.height * scale;

	Image toReturn = { std::vector<unsigned char>(dst_width*dst_height * 4), dst_width, dst_height };

	const unsigned char* src = &img.img[0];
	unsigned char* dst = &toReturn.img[0];

	for (unsigned int color = 0; color < 4; color++) {
		for (unsigned int x = 0; x < dst_width; x++) {
			for (unsigned int y = 0; y < dst_height; y++) {

				unsigned int x0 = x / scale;
				unsigned int y0 = y / scale;

				unsigned char rightBot = src[4 * (x0 + y0*src_width) + color];
				unsigned char rightUp = src[4 * (x0 + (y0 + 1)*src_width) + color];
				unsigned char leftBot = src[4 * (x0 + 1 + y0*src_width) + color];
				unsigned char leftUp = src[4 * (x0 + 1 + (y0 + 1)*src_width) + color];

				float x_ratio = ((float)x) / scale - x0;
				float y_ratio = ((float)y) / scale - y0;

				float leftUpRatio = x_ratio*y_ratio;
				float leftBotRatio = x_ratio*(1-y_ratio);
				float rightUpRatio = (1-x_ratio)*y_ratio;
				float rightBotRatio = (1-x_ratio)*(1 - y_ratio);

				float value = leftUpRatio * leftUp + leftBotRatio * leftBot + rightUpRatio * rightUp + rightBotRatio * rightBot;
				dst[4 * (x + y*dst_width) + color] = value;
			}
		}
	}

	return toReturn;
}

unsigned int clamp(unsigned int value, unsigned int min, unsigned int max) {
	if (value < min) { return min; }
	else if (value > max) { return max; }
	else { return value; }
}

Image processBlur(const Image& src, int size, int compare(int src, int dst) ) {

	unsigned int width = src.width;
	unsigned int height = src.height;
	Image toReturn = { std::vector<unsigned char>( width * height * 4), width, height };

	for (unsigned int color = 0; color < 4; color++) {
		for (unsigned int x0 = 0; x0 < width; x0++) {
			for (unsigned int y0 = 0; y0 < height; y0++) {

				unsigned int sum = 0;
				unsigned char pixel = src.img[4 * (x0 + y0*width) + color];
				for (unsigned int x = x0 - size; x < x0 + size; x++) {
					for (unsigned int y = y0 - size; y < y0 + size; y++) {

						unsigned int x_clamp = clamp(x, 0, width-1);
						unsigned int y_clamp = clamp(y, 0, height-1);
						sum += compare( pixel, src.img[4 * (x_clamp + width*y_clamp) + color]);
					}
				}
				toReturn.img[4 * (x0 + width*y0) + color] = sum / (4 * size*size);
			}
		}
	}

	return toReturn;
}

Image scalePaint(const Image& src, int scale) {

	Image up = upscaleLinear(src, scale);
	Image blur = processBlur(up, scale, [](int src, int dst) { return dst; } );
	Image blurHigh = processBlur(up, scale, [](int src, int dst) { return (src < dst) ? dst : src; });
	Image blurLow = processBlur(up, scale, [](int src, int dst) { return (src > dst) ? dst : src; });

	Image toReturn = { std::vector<unsigned char>(up.width * up.height * 4), up.width, up.height };
	for (unsigned int i = 0; i < 4 * up.width*up.height; i++) {
		
		toReturn.img[i] = (up.img[i] > blur.img[i]) ? blurHigh.img[i] : blurLow.img[i];
	}

	return toReturn;
}