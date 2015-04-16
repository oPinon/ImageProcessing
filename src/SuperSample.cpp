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
			printf("%u % ", perc);
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
float kernelSinc(float x) { // lanczos
	if (x == 0) { return 1; }
	float xAbs = abs(x);// if (xAbs > 2) { return 0; }
	return (float) ( sin(xAbs*3.1416) * sin(xAbs*3.1416/2) / (xAbs*xAbs) );
}
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
float kernelCubic1313(float x) {
	x = abs(x);
	if (x >= 2) { return 0; }
	else if (x >= 1) {
		return 1 / 6.0f * (-2.3333f*x*x*x + 12 * x*x - 20 * x + 10.666f);
	}
	else {
		return 1 / 6.0f * (7 * x*x*x - 12 * x*x + 5.3333f);
	}
} // Mitchell

Image resize(const Image& src, const uint dstWidth, const uint dstHeight, Interpolation interpolation) {

	auto kernel = kernelNN;
	switch (interpolation) {
		case NEAREST: kernel = kernelNN; KernelSpan = 1.0f; break;
		case LINEAR: kernel = kernelLin; KernelSpan = 1.0f; break;
		case LANCZOS: kernel = kernelSinc; KernelSpan = 2.0f; break;
		case CUBIC: kernel = kernelCubic1313; KernelSpan = 2.0f; break;
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
	dst.img = (uchar*)malloc(dst.channels*dst.width*dst.height*sizeof(uchar));

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
	free(temp);
	return dst;
}

// indexes of the filters
int sizeFilters = 7;
int filterIndexes[] = { -3, -2, -1, 0, 1, 2, 3 };
// filters for the 5:4 downsampling
float d0_5_4[] = { -0.013, -0.017, 0.074, 0.804, 0.185, -0.045, 0.011 };
float d1_5_4[] = { -0.005, 0.032, -0.129, 0.753, 0.421, -0.09, 0.017 };
float d2_5_4[] = { 0.017, -0.09, 0.421, 0.753, -0.129, 0.032, -0.005 };
float d3_5_4[] = { 0.011, -0.045, 0.185, 0.804, 0.074, -0.017, -0.013 };

float* down_5_4(const float* src, const uint size) {

	uint newSize = (size * 4) / 5;
	float* dst = (float*)malloc(newSize*sizeof(float));
	for (int i = 0; i < newSize; i++) { dst[i] = 0; }
	for (int i = 0; i < size / 5; i++) {
		for (int j = 0; j < 4; j++) {
			float* filter = nullptr;
			if (j == 0) { filter = d0_5_4; }
			if (j == 1) { filter = d1_5_4; }
			if (j == 2) { filter = d2_5_4; }
			if (j == 3) { filter = d3_5_4; }
			int highIndex = 5 * i + ( j < 2 ? j : j + 1);
			int lowIndex = 4 * i + j;
			float sum = 0; float norm = 0;
			for (int k = 0; k < sizeFilters; 
				k++) {
				int index = highIndex + filterIndexes[k];
				if (index < 0 || index >= size) { continue; }
				norm += filter[k];
				sum += filter[k] * src[index];
			}
			dst[lowIndex] = sum / norm;
		}
	}
	return dst;
}

Image down_5_4(const Image& src) {

	uint w = src.width; uint h = src.height;
	uint c = src.channels;

	uint w2 = (w * 4) / 5; uint h2 = (h * 4) / 5;

	float* temp = (float*)malloc(c * w * h2 * sizeof(float));

	for (uint k = 0; k < c; k++) {
		for (uint x = 0; x < w; x++) {
			float* column = (float*)malloc(h*sizeof(float));
			for (uint y = 0; y < h; y++) {
				column[y] = src.img[c*(w*y + x) + k];
			}
			float* newColumn = down_5_4(column, h);
			for (uint y = 0; y < h2; y++) {
				temp[c*(w*y + x) + k] = newColumn[y];
			}
			free(column);
			free(newColumn);
		}
	}

	Image dst;
	dst.channels = c;
	dst.width = w2;
	dst.height = h2;
	dst.img = (uchar*)malloc(dst.channels*dst.width*dst.height*sizeof(uchar));

	for (uint k = 0; k < c; k++) {
		for (uint y = 0; y < h2; y++) {
			float* row = (float*)malloc(w*sizeof(float));
			for (uint x = 0; x < w; x++) {
				row[x] = temp[c*(w*y + x) + k];
			}
			float* newRow = down_5_4(row, w);
			for (uint x = 0; x < w2; x++) {
				dst.img[c*(w2*y + x) + k] = clamp(newRow[x]);
			}
			free(row);
			free(newRow);
		}
	}
	free(temp);
	return dst;
}

float u0_5_4[] = { -0.028, -0.053, 0.061, 0.925, 0.304, 0.007, 0.014 };
float u1_5_4[] = { 0, 0.038, -0.086, 0.862, 0.52, -0.128, 0.062 };
float u2_5_4[] = { 0.062, -0.128, 0.52, 0.862, -0.086, 0.038, 0 };
float u3_5_4[] = { 0.014, 0.007, 0.304, 0.925, 0.061, -0.053, -0.028 };

float* up_5_4(const float* src, const uint size) {

	uint newSize = (size * 5) / 4;
	float* dst = (float*)malloc(newSize*sizeof(float));
	float* sum = (float*)malloc(newSize*sizeof(float));
	float* norm = (float*)malloc(newSize*sizeof(float));
	for (int i = 0; i < newSize; i++) { dst[i] = 0; sum[i] = 0; norm[i] = 0; }
	for (int i = 0; i < size / 4; i ++) {
		for (int j = 0; j < 5; j++) {
			if (j == 2) { continue; }
			float* filter = nullptr;
			if (j == 0) { filter = u0_5_4; }
			if (j == 1) { filter = u1_5_4; }
			if (j == 3) { filter = u2_5_4; }
			if (j == 4) { filter = u3_5_4; }
			int highIndex = 5 * i + j;
			int lowIndex = 4 * i + (j < 2 ? j : j - 1);
			float value = src[lowIndex];
			for (int k = 0; k < sizeFilters; k++) {
				int index = highIndex + filterIndexes[k];
				if (index < 0 || index >= newSize) { continue; }
				sum[index] += filter[k] * value;
				norm[index] += filter[k];
			}
		}
	}
	for (int i = 0; i < newSize; i++) { dst[i] = sum[i] / norm[i]; }
	free(sum); free(norm);
	return dst;
}

Image up_5_4(const Image& src) {

	uint w = src.width; uint h = src.height;
	uint c = src.channels;

	uint w2 = (w * 5) / 4; uint h2 = (h * 5) / 4;

	float* temp = (float*)malloc(c * w * h2 * sizeof(float));

	for (uint k = 0; k < c; k++) {
		for (uint x = 0; x < w; x++) {
			float* column = (float*)malloc(h*sizeof(float));
			for (uint y = 0; y < h; y++) {
				column[y] = src.img[c*(w*y + x) + k];
			}
			float* newColumn = up_5_4(column, h);
			for (uint y = 0; y < h2; y++) {
				temp[c*(w*y + x) + k] = newColumn[y];
			}
			free(column);
			free(newColumn);
		}
	}

	Image dst;
	dst.channels = c;
	dst.width = w2;
	dst.height = h2;
	dst.img = (uchar*)malloc(dst.channels*dst.width*dst.height*sizeof(uchar));

	for (uint k = 0; k < c; k++) {
		for (uint y = 0; y < h2; y++) {
			float* row = (float*)malloc(w*sizeof(float));
			for (uint x = 0; x < w; x++) {
				row[x] = temp[c*(w*y + x) + k];
			}
			float* newRow = up_5_4(row, w);
			for (uint x = 0; x < w2; x++) {
				dst.img[c*(w2*y + x) + k] = clamp(newRow[x]);
			}
			free(row);
			free(newRow);
		}
	}
	free(temp);
	return dst;
}
