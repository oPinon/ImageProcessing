#include "SuperSample.h"
#include "lodepng.h"

#include <stdlib.h>

uint patchSize = 5;
uint searchSize = 5;

template<class type>
uchar clamp(type v) {
	if (v < 0) { return 0; }
	if (v > 255) { return 255; }
	return (uchar) v;
}

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

	uint last = 0;
	// for all dense patches in the hires image
	for (uint x0 = 0; x0 < highWidth - patchSize; x0++) {

		/*uint perc = (x0 * 100) / (highWidth - patchSize);
		if (perc != last) {
			printf("%u % \n", perc);
			last = perc;
		}*/

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
		dst.img[i] = clamp( pix );
	}

	free(diff.img);
	free(diff.count);

	return dst;
}

Image loadImage(const char* filename) {

	Image dst;
	dst.channels = 4;
	uint error = lodepng_decode32_file( &dst.img, &dst.width, &dst.height, filename);
	if (error)  printf("error %u: %s\n", error, lodepng_error_text(error));
	return dst;
}

void writeImage(Image im, const char* filename) {

	uint error = lodepng_encode32_file(filename, im.img, im.width, im.height);
	if (error) printf("error %u: %s\n", error, lodepng_error_text(error));
}

uint max(const uint a, const uint b) { if (a > b) { return a; } else { return b; } }
float max(const float a, const float b) { if (a > b) { return a; } else { return b; } }
uint min(const uint a, const uint b) { if (a < b) { return a; } else { return b; } }

static float KernelSpan = 2.0f; // 2 works for all kernels. Linear requires 1, NN requires 0.5. Decrease for faster computations.

float kernelNN(float x) { float xAbs = abs(x); return (xAbs < 0.5) ? 1.0f : 0.0f; } // nearest neighbor
float kernelLin(float x) { float xAbs = abs(x); return (xAbs < 1) ? 1 - xAbs : 0; } // linear
float kernelSinc(float x) { if (x == 0) { return 1; } float xAbs = abs(x); return sin(xAbs*3.14) / (3.14*xAbs); } // sinc, 2 lobes
float kernelCubic10(float x) {
	x = abs(x);
	if (x >= 2) { return 0; }
	else if (x >= 1) {
		return 1 / 6.0f * (-x*x*x + 6 * x*x - 12 * x + 8);
	}
	else {
		return 1 / 6.0f * (3 * x*x*x - 6 * x*x + 4);
	}
} // cubic smoother

Image resize(const Image& src, const uint dstWidth, const uint dstHeight, Interpolation interpolation) {

	auto kernel = kernelNN;
	switch (interpolation) {
		case NEAREST: kernel = kernelNN; KernelSpan = 1.0f; break;
		case LINEAR: kernel = kernelLin; KernelSpan = 1.0f; break;
		case SINC: kernel = kernelSinc; KernelSpan = 2.0f; break;
		case CUBIC10: kernel = kernelCubic10; KernelSpan = 2.0f; break;
	}

	float kernelScaling = ((float)(src.height) / min(dstHeight, src.height)); // scales the kernel (on the src ref) to the size of the biggest. = 1 if upscaling, > 1 otherwise
	uint kernelSpan = (uint) (2 * KernelSpan * kernelScaling);

	float* coeffs = (float*)malloc(dstHeight*kernelSpan*sizeof(float));
	uint* starts = (uint*)malloc(dstHeight*sizeof(uint));

	for (uint i = 0; i < dstHeight; i++) {

		float yDst = ((float)(i + 0.5f)*src.height) / dstHeight;
		uint start = (uint)ceil(max(0, yDst - 0.5f - kernelSpan / 2)); // ceil -> fist int greater than or equal
		starts[i] = start;
		for (uint j = 0; j < kernelSpan; j++) {
			float ySrc = start + j + 0.5f;
			float coeff = kernel((yDst - ySrc) / kernelScaling);
			coeffs[i*kernelSpan + j] = coeff;
		}
	}

	float* temp = (float*) malloc(src.channels*src.width*dstHeight*sizeof(float));

	// TODO : alpha should be used as an interpolation factor
	for (uint k = 0; k < src.channels; k++) {
		for (uint x = 0; x < src.width; x++) {
			for (uint y = 0; y < dstHeight; y++) {

				float sum = 0, norm = 0;
				for (uint i = 0; i < kernelSpan; i++) {
					uint srcIndex = src.channels*(src.width*max(0, min(starts[y] + i, src.height - 1))+x) + k;
					float coeff = coeffs[y*kernelSpan + i];
					norm += coeff;
					sum += src.img[srcIndex] * coeff;
				}
				temp[src.channels*(src.width*y + x) + k] = sum / norm;
			}
		}
	}

	free(coeffs);
	free(starts);

	// the same but for horizontal scaling

	kernelScaling = ((float)(src.width) / min(dstWidth, src.width));
	kernelSpan = (uint) ( 2 * KernelSpan * kernelScaling ); // total span of the kernel

	coeffs = (float*)malloc(dstWidth*kernelSpan*sizeof(float));
	starts = (uint*)malloc(dstWidth*sizeof(uint));

	for (uint i = 0; i < dstWidth; i++) {

		float xDst = ((float)(i + 0.5f)*src.width) / dstWidth;
		uint start = (uint) ceil( max(0, xDst - 0.5f - kernelSpan/2) ); // ceil -> fist int greater than or equal
		//cout << "start = " << start << endl;
		starts[i] = start;
		for (uint j = 0; j < kernelSpan; j++) {
			float xSrc = start + j + 0.5f;
			float coeff = kernel((xDst - xSrc) / kernelScaling);
			coeffs[i*kernelSpan + j] = coeff;
		}
	}

	Image dst;
	dst.channels = src.channels;
	dst.width = dstWidth;
	dst.height = dstHeight;
	dst.img = (uchar*)malloc(dst.channels*dst.width*dst.height);

	// TODO : alpha should be used as an interpolation factor
	for (uint k = 0; k < dst.channels; k++) {
		for (uint y = 0; y < dst.height; y++) {
			for (uint x = 0; x < dst.width; x++) {

				float sum = 0, norm = 0;
				for (uint i = 0; i < kernelSpan; i++) {
					uint srcIndex = src.channels*(src.width*y + max(0,min(starts[x] + i, src.width-1))) + k;
					float coeff = coeffs[x*kernelSpan + i];
					norm += coeff;
					sum += temp[srcIndex] * coeff;
				}
				dst.img[dst.channels*(dst.width*y + x) + k] = clamp(sum / norm);
			}
		}
	}

	free(coeffs);
	free(starts);
	return dst;
}