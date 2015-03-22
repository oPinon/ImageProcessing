#include "SuperSample.h"
#include "lodepng.h"

#include <stdlib.h>

uint patchSize = 5;
uint searchSize = 5;

Image superSample(Image src, Image srcB, Image dstB) {

	uint highWidth = dstB.width;
	uint highHeight = dstB.height;

	uint lowWidth = src.width;
	uint lowHeight = src.height;
	// error if src & srcB have != width or height

	uint channels = src.channels;
	// error if src & srcB & dstB have != channels

	ImageD diff; // difference image between dstB(lurry) and dst. Pixels will have to be averaged
	diff.channels = channels;
	diff.width = highWidth;
	diff.height = highHeight;
	uint length = diff.channels * diff.width * diff.height;
	diff.img = (int*) malloc( length*sizeof(int) );
	diff.count = (uint*) malloc( length*sizeof(uint) );
	for (uint i = 0; i < length; i++) { 
		diff.img[i] = 0;
		diff.count[i] = 0;
	}

	// for all dense patches in the hires image
	for (uint x0 = 0; x0 < highWidth - patchSize; x0++) {

		for (uint y0 = 0; y0 < highHeight - patchSize; y0++) {

			uint minDist = UINT_MAX; // compute the closest patch
			uint bestPatchX = 0;
			uint bestPatchY = 0;

			uint x1 = (x0*lowWidth) / highWidth; // corresponding lowres position
			uint y1 = (y0*lowHeight) / highHeight;

			// for all near patches in the lowres image
			for (int x = -(int)searchSize; x < (int)searchSize; x++) {

				for (int y = -(int)searchSize; y < (int)searchSize; y++) {

					int x2 = x1 + x; // position of the lowres patch
					int y2 = y1 + y;
					if (x2 < 0 || x2 >= lowWidth - patchSize) { continue; }
					if (y2 < 0 || y2 >= lowHeight - patchSize) { continue; }

					uint dist = 0;
					// for all pixels of the lowres patch add the distance between dstB and srcB
					for (uint k = 0; k < channels; k++) {
						for (uint x3 = 0; x3 < patchSize; x3++) {
							for (uint y3 = 0; y3 < patchSize; y3++) {

								uint indexLow = channels * (lowWidth * (y2 + y3) + x2 + x3) + k; // pixel at the lowres patch
								uint indexHigh = channels * (highWidth * (y0 + y3) + x0 + x3) + k; // pixel at the highres patch
								int d = ((int)dstB.img[indexHigh]) - ((int)srcB.img[indexLow]);
								dist += d*d;
							}
						}
					}
					if (dist <= minDist) { 
						minDist = dist;
						bestPatchX = (uint) x2;
						bestPatchY = (uint) y2;
					}
				}
			}

			// add the high frequency details to the diff image
			for (uint k = 0; k < channels; k++) {
				for (uint x = 0; x < patchSize; x++) {
					for (uint y = 0; y < patchSize; y++) {

						uint indexLow = channels*(lowWidth*(bestPatchY + y) + x + bestPatchX) + k;
						int d = ((int) src.img[indexLow]) - ((int) srcB.img[indexLow]);

						uint indexHigh = channels*(diff.width*(y0+y) + x0+x) + k;
						diff.img[indexHigh] += d;
						diff.count[indexHigh] += 1;
					}
				}
			}
		}
	}

	Image dst;
	dst.channels = channels;
	dst.width = highWidth;
	dst.height = highHeight;
	dst.img = (uchar*) malloc(channels*dst.width*dst.height*sizeof(uchar));

	for (uint i = 0; i < channels * dst.width * dst.height; i++) {
		uint count = diff.count[i];
		int d = 0;
		if (count > 0) { d = diff.img[i] / ((int) count);  }
		int pix = dstB.img[i] + d;
		if (pix < 0) { pix = 0; }
		if (pix > 255) { pix = 255; }
		dst.img[i] = (uchar) pix;
	}

	free(diff.img);
	free(diff.count);

	return dst;
}

Image loadImage(char* filename) {

	Image dst;
	dst.channels = 4;
	uint error = lodepng_decode32_file( &dst.img, &dst.width, &dst.height, filename);
	if (error)  printf("error %u: %s\n", error, lodepng_error_text(error));
	return dst;
}

void writeImage(Image im, char* filename) {

	uint error = lodepng_encode32_file(filename, im.img, im.width, im.height);
	if (error) printf("error %u: %s\n", error, lodepng_error_text(error));
}