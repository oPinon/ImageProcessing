// standard libraries
#include <iostream>
#include <time.h>

// project libraries
#include "SuperSample.h"

using namespace std;

int main(int argc, char** argv) {

	Image src = loadImage("../images/input_small.png");

	float step = 1.25;
	uint width = 400, height = 400;
	Interpolation down = LINEAR;
	Interpolation up = LINEAR;

	//Image srcB = resize( resize(src, src.width / step, src.height / step, down), src.width, src.height, up);
	Image srcB = up_5_4(down_5_4(src));

	Image dst = src;

	do {
		//Image dstB = resize(dst, dst.width*step, dst.height*step, up);
		Image dstB = up_5_4(dst);
		dst = superSample(src, srcB, dstB);
		cout << " [ " << dst.width << "; " << dst.height << " ]" << endl;
		
	} while (dst.width < width || dst.height < height);

	//Image dst = resize(resize(src, src.width / step, src.height / step, down), src.width, src.height, up);

	//Image dst = up_5_4( down_5_4(src) );

	writeImage(dst, "../images/dst.png");

	return 0;
}