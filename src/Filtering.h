#pragma once

#include <vector>
#include "Image.h"

Image processKernel(Image& src, unsigned int kernelSize, void enter(unsigned char* pixel), void exit(unsigned char* pixel), void set(unsigned char* pixel));

Image blur(Image& src, int k);