#pragma once

#include <libgimp/gimp.h>

#include "GimpPlugin.h"

// a float image
typedef struct _ImageF {
	float* img;
	gint width, height, channels;
} ImageF;

// a guchar image
typedef struct _Image {
	guchar* img; // array of pixels : RGBARGBA... for example
	gint width, height, channels;
} Image;

// convert an image from guchar to float, and back
ImageF charToFloat(Image src);
Image floatToChar(ImageF src);

// main function, used in the "run" function
Image supersample(guchar* src, gint srcW, gint srcH, gint channels, FilterValues fvalues);

// analytical resizing using an interpolation kernel
ImageF resize(ImageF src, gint dstW, gint dstH, Kernel interpolation);

// predicts the high resolution details using a self-similarity algorithm
ImageF predict(ImageF src, ImageF srcB, ImageF dstB);