#pragma once

#include <vector>
#include "Image.h"

Image scaleNearest(const Image& src, float scale);

Image upscaleLinear(const Image& src, float scale);

Image scalePaint(const Image& src, int scale);