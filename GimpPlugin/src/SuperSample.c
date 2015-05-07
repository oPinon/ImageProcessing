#include "SuperSample.h"

gint patchSize = 5;
gint searchSize = 2;

gint toProcess;
gint processed;

ImageF copy(ImageF src) {

	ImageF dst;
	dst.channels = src.channels;
	dst.width = src.width; dst.height = src.height;
	dst.img = g_new(float, src.channels*src.width*src.height);
	gint i;
	for (i = 0; i < src.channels*src.width*src.height; i++) { dst.img[i] = src.img[i]; }
	return dst;
}

Image supersample(guchar* srcImg, gint srcW, gint srcH, gint channels, FilterValues fvalues) {

	gint width = fvalues.width, height = fvalues.height;

	Image srcC;
	srcC.img = srcImg;
	srcC.width = srcW; srcC.height = srcH;
	srcC.channels = channels;

	float step = 1.25;

	toProcess = 0; processed = 0;
	gint w = srcW, h = srcH;
	while (w < width || h < height) {
		w *= step;
		h *= step;
		toProcess += w;
	}


	ImageF src = charToFloat(srcC);

	ImageF low = resize(src, src.width / step, src.height / step);
	ImageF srcB = resize(low, src.width, src.height);

	g_free(low.img);

	ImageF dst = copy(src);

	while (dst.width < width || dst.height < height) {

		ImageF dstB = resize(dst, dst.width*step, dst.height*step);
		g_free(dst.img);
		dst = predict(src, srcB, dstB);
		g_free(dstB.img);

	}

	Image dstU = floatToChar(dst);

	g_free(srcB.img);
	g_free(dst.img);
	g_free(src.img);

	return dstU;
}

/*float max(const float a, const float b) { if (a > b) { return a; } else { return b; } }
gint min(const gint a, const gint b) { if (a < b) { return a; } else { return b; } }*/
float Abs(float x) { return x >= 0 ? x : -x; }

/*float kernel(float x) { // Linear
float xAbs = Abs(x); return xAbs < 1 ? 1 - xAbs : 0;
}*/
/*float kernel(float x) { // Lanczos
	if (x == 0) { return 1; }
	float xAbs = Abs(x);
	return (float)(sin(xAbs*3.1416) * sin(xAbs*3.1416 / 2) / (xAbs*xAbs));
	}*/
float kernel(float x) { // Mitchell (Cubic1313)
	x = Abs(x);
	if (x >= 2) { return 0; }
	else if (x >= 1) {
		return 1 / 6.0f * (-2.3333f*x*x*x + 12 * x*x - 20 * x + 10.666f);
	}
	else {
		return 1 / 6.0f * (7 * x*x*x - 12 * x*x + 5.3333f);
	}
}

ImageF resize(ImageF src, gint dstW, gint dstH) {

	float KernelSpan = 2.0f;
	float kernelScaling = ((float)(src.height) / min(dstH, src.height));
	gint kernelSpan = (gint)(2 * KernelSpan * kernelScaling);

	float* coeffs = g_new(float, dstH*kernelSpan);
	gint* starts = g_new(gint, dstH);

	gint i, j;
	for (i = 0; i < dstH; i++) {

		float yDst = ((float)(i + 0.5f)*src.height) / dstH;
		gint start = (gint)(max(0, yDst - 0.5f - kernelSpan / 2) + 0.999f);
		starts[i] = start;
		for (j = 0; j < kernelSpan; j++) {
			float ySrc = start + j + 0.5f;
			float coeff = kernel((yDst - ySrc) / kernelScaling);
			coeffs[i*kernelSpan + j] = coeff;
		}
	}

	float* temp = g_new(float, src.channels*src.width*dstH);

	gint k, x, y;
	for (x = 0; x < src.width; x++) {
		for (y = 0; y < dstH; y++) {
			for (k = 0; k < src.channels; k++) {

				float sum = 0, norm = 0;
				for (i = 0; i < kernelSpan; i++) {
					gint srcIndex = src.channels*(src.width*max(0, min(starts[y] + i, src.height - 1)) + x) + k;
					float coeff = coeffs[y*kernelSpan + i];
					norm += coeff;
					sum += src.img[srcIndex] * coeff;
				}
				temp[src.channels*(src.width*y + x) + k] = sum / norm;
			}
		}
	}

	g_free(coeffs);
	g_free(starts);

	kernelScaling = ((float)(src.width) / min(dstW, src.width));
	kernelSpan = (gint)(2 * KernelSpan * kernelScaling); // total span of the kernel

	coeffs = g_new(float, dstW*kernelSpan);
	starts = g_new(gint, dstW);

	for (i = 0; i < dstW; i++) {

		float xDst = ((float)(i + 0.5f)*src.width) / dstW;
		gint start = (gint)(max(0, xDst - 0.5f - kernelSpan / 2) + 0.999f);
		starts[i] = start;
		for (j = 0; j < kernelSpan; j++) {
			float xSrc = start + j + 0.5f;
			float coeff = kernel((xDst - xSrc) / kernelScaling);
			coeffs[i*kernelSpan + j] = coeff;
		}
	}

	float* dst = g_new(float, src.channels * dstW * dstH);

	for (y = 0; y < dstH; y++) {
		for (x = 0; x < dstW; x++) {
			for (k = 0; k < src.channels; k++) {

				float sum = 0, norm = 0;
				for (i = 0; i < kernelSpan; i++) {
					gint srcIndex = src.channels*(src.width*y + max(0, min(starts[x] + i, src.width - 1))) + k;
					float coeff = coeffs[x*kernelSpan + i];
					norm += coeff;
					sum += temp[srcIndex] * coeff;
				}
				dst[src.channels*(dstW*y + x) + k] = sum / norm;
			}
		}
	}

	g_free(coeffs);
	g_free(starts);
	g_free(temp);

	ImageF dstIm;
	dstIm.width = dstW; dstIm.height = dstH;
	dstIm.channels = src.channels;
	dstIm.img = dst;

	return dstIm;

}

ImageF charToFloat(Image src) {

	ImageF dst;
	dst.width = src.width; dst.height = src.height;
	dst.channels = src.channels;
	gint size = src.channels * src.width * src.height;
	dst.img = g_new(float, size);
	gint i;
	for (i = 0; i < size; i++) { dst.img[i] = src.img[i]; }
	return dst;
}

// converts a float to a uchar
guchar clampF(float x) {
	if (x < 0) { return 0; }
	else if (x > 255) { return 255; }
	else { return (guchar)x; }
}

Image floatToChar(ImageF src) {

	Image dst;
	dst.width = src.width; dst.height = src.height;
	dst.channels = src.channels;
	gint size = src.channels * src.width * src.height;
	dst.img = g_new(guchar, size);
	gint i;
	for (i = 0; i < size; i++) { dst.img[i] = clampF(src.img[i]); }
	return dst;
}

typedef struct _ImageD {
	double* img; // sum of values of each pixels
	gint* count; // nb of values for each pixels (for future averaging)
	gint width, height, channels;
} ImageD;

ImageF predict(ImageF src, ImageF srcB, ImageF dstB) {

	int i, x0, y0, x, y, k, x3, y3;
	gint highWidth = dstB.width;
	gint highHeight = dstB.height;

	gint lowWidth = srcB.width;
	gint lowHeight = srcB.height;

	gint channels = src.channels;

	ImageD diff; // difference image between dstB(lurry) and dst. Pixels will have to be averaged
	diff.channels = channels;
	diff.width = highWidth;
	diff.height = highHeight;
	gint length = diff.channels * diff.width * diff.height;
	diff.img = g_new(double, length);
	diff.count = g_new(gint, length);
	for (i = 0; i < length; i++) {
		diff.img[i] = 0;
		diff.count[i] = 0;
	}

	gint last = 0;

	for (x0 = 0; x0 < highWidth - patchSize; x0++) {

		gimp_progress_update(((gdouble)processed) / toProcess);
		processed++;

		for (y0 = 0; y0 < highHeight - patchSize; y0++) {

			double minDist = INFINITY;

			gint bestPatchX = 0;
			gint bestPatchY = 0;

			gint x1 = (x0*lowWidth) / highWidth; // corresponding lowres position
			gint y1 = (y0*lowHeight) / highHeight;

			// for all near patches in the lowres image
			for (x = -(int)searchSize; x < (int)searchSize && minDist != 0; x++) {

				for (y = -(int)searchSize; y < (int)searchSize && minDist != 0; y++) {

					int x2 = x1 + x; // position of the lowres patch
					int y2 = y1 + y;
					if (x2 < 0 || x2 >= lowWidth - patchSize) { continue; }
					if (y2 < 0 || y2 >= lowHeight - patchSize) { continue; }

					double dist = 0;
					// for all pixels of the lowres patch add the distance between dstB and srcB
					for (k = 0; k < channels; k++) {
						for (x3 = 0; x3 < patchSize; x3++) {
							for (y3 = 0; y3 < patchSize; y3++) {

								gint indexLow = channels * (lowWidth * (y2 + y3) + x2 + x3) + k; // pixel at the lowres patch
								gint indexHigh = channels * (highWidth * (y0 + y3) + x0 + x3) + k; // pixel at the highres patch
								float d = dstB.img[indexHigh] - srcB.img[indexLow];
								dist += d*d;
							}
						}
					}
					if (dist <= minDist) {
						minDist = dist;
						bestPatchX = (gint)x2;
						bestPatchY = (gint)y2;
					}
				}
			}

			// add the high frequency details to the diff image
			for (k = 0; k < channels; k++) {
				for (x = 0; x < patchSize; x++) {
					for (y = 0; y < patchSize; y++) {

						// Warning : src and srcB might have some slightly different sizes, due to rounding
						gint indexSRC = channels*(src.width*(bestPatchY + y) + x + bestPatchX) + k;
						gint indexSRCB = channels*(srcB.width*(bestPatchY + y) + x + bestPatchX) + k;
						float d = src.img[indexSRC] - srcB.img[indexSRCB];

						gint indexHigh = channels*(diff.width*(y0 + y) + x0 + x) + k;
						diff.img[indexHigh] += d;
						diff.count[indexHigh] += 1;
					}
				}
			}
		}
	}

	ImageF dst;
	dst.channels = channels;
	dst.width = highWidth;
	dst.height = highHeight;
	dst.img = g_new(float, channels*dst.width*dst.height);

	for (i = 0; i < channels * dst.width * dst.height; i++) {
		gint count = diff.count[i];
		int d = 0;
		if (count > 0) { d = diff.img[i] / ((int)count); }
		int pix = dstB.img[i] + d;
		dst.img[i] = pix;
	}

	g_free(diff.img);
	g_free(diff.count);

	return dst;
}