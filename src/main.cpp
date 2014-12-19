// standard libraries
#include <iostream>
#include <time.h>

// external libraries
#include "lodepng.h"

// project libraries
#include "Blur.h"

using namespace std;

struct Image {
	// RGBA RGBA RGBA. y*width + x
	vector<unsigned char> img;
	unsigned int width, height;
};

int main(int argc, char* argv) {

	char* src_filename = "../images/mona.png";
	char* out_filename = "../images/out.png";

	// reading input image
	Image src;
	unsigned error = lodepng::decode(src.img, src.width, src.height, src_filename);
	if (error) { cerr << "error " << error << " when opening " << src_filename << " : " << lodepng_error_text(error) << endl; return 1; }
	else { cout << "opened " << src_filename << " [ " << src.width << " ; " << src.height << " ] " << endl; }
	Image dst(src);

	clock_t start = clock();

	// Processing
	blur(&src.img[0], &dst.img[0], src.width, src.height, 10);

	cout << "process done in " << (clock() - start) * 1000. / CLOCKS_PER_SEC << " ms" << endl;

	// writing output image
	error = lodepng::encode(out_filename, dst.img, dst.width, dst.height);
	if (error) { cerr << "error " << error << " when writing " << out_filename << " : " << lodepng_error_text(error) << endl; return 1; }

	return 0;
}