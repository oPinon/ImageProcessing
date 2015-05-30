#include "SuperSample.h"
#include <vector>

struct Pixel {
	float* pix;
	float *up, *left, *bottom, *right;
	float laplacian[3];
	Pixel(uint x, uint y, const ImageF& img) {

		uint pixI = img.channels*(img.width*y + x);
		uint upI = img.channels*(img.width*(y - 1) + x);
		uint leftI = img.channels*(img.width*y + x - 1);
		uint bottomI = img.channels*(img.width*(y + 1) + x);
		uint rightI = img.channels*(img.width*y + x + 1);

		pix = img.img + pixI;
		up = img.img + upI; left = img.img + leftI;
		bottom = img.img + bottomI; right = img.img + rightI;
	}

	Pixel(uint x, uint y, const ImageF& img, const ImageF& target) : Pixel(x, y, img) {

		uint pixI = target.channels*(target.width*y + x);
		uint upI = target.channels*(target.width*(y - 1) + x);
		uint leftI = target.channels*(target.width*y + x - 1);
		uint bottomI = target.channels*(target.width*(y + 1) + x);
		uint rightI = target.channels*(target.width*y + x + 1);

		for (uint k = 0; k < 3; k++) {
			laplacian[k] = target.img[pixI + k] - (target.img[leftI + k] + target.img[upI + k] + target.img[bottomI + k] + target.img[rightI + k]) / 4;
		}

	}
};

Image inpainting(const Image& src0, const Image* target0 = NULL, float threshold = 1) {

	bool hasTarget = target0 != NULL;
	ImageF src = convert(src0);
	ImageF target; if (hasTarget) { target = convert(*target0); }

	vector<Pixel> pixels;
	for (uint x = 0; x < src.width; x++) {
		for (uint y = 0; y < src.height; y++) {
			auto pix = src.img + src.channels*(src.width*y + x);
			auto alpha = pix[3];
			if (alpha < 255) {
				pixels.push_back( hasTarget ? Pixel(x, y, src, target) : Pixel(x,y,src) );
				pix[3] = 255;
			}
		}
	}
	//cout << "Working on " << pixels.size() << " pixels" << endl;

	float diff = INFINITY;
	while (diff > threshold) {

		diff = 0;
		for (const Pixel& p : pixels) {
			for (uint k = 0; k < 3; k++) {
				float newPix = p.laplacian[k] + (p.left[k] + p.bottom[k] + p.right[k] + p.up[k]) / 4;
				diff += abs(newPix - p.pix[k]);
				p.pix[k] = newPix;
			}
		}
	}

	Image dst = convert(src); free(src.img);
	return dst;
}

void inpainting(int argc, char** argv) {


}