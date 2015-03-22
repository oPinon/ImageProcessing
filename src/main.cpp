// standard libraries
#include <iostream>
#include <time.h>

// project libraries
#include "SuperSample.h"

using namespace std;

int main(int argc, char** argv) {

	Image src = loadImage("../images/SuperSample/128.png");
	Image srcB = loadImage("../images/SuperSample/128l.png");
	Image dstB = loadImage("../images/SuperSample/256.png");

	printf("computing supersampling... ");
	clock_t start = clock();

	Image dst = superSample(src, srcB, dstB);

	printf("done in %u ms\n", (unsigned int) (clock() - start)* 1000 / CLOCKS_PER_SEC);

	writeImage(dst, "../images/SuperSample/dst.png");

	return 0;
}