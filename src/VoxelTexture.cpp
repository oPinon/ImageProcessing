#include "VoxelTexture.h"

using namespace std;

void paste(Image& src, Image& dst, int x, int y) {

	for (unsigned int color = 0; color < 4; color++) {
		for (unsigned int i = 0; i < src.width; i++) {
			for (unsigned int j = 0; j < src.height; j++) {
				dst.img[4 * ((x + i) + dst.width*(j + y)) + color] = src.img[4 * (i + src.width*j) + color];
			}
		}
	}
};

Image loadVoxelImages(string folder, int digits_nb) {

	vector<Image> images; // array of images

	int index = (int) pow(10, digits_nb + 1); // the index of images (must remove the first "1")
	index += 1;
	
	Image firstImage = loadImage(folder + to_string(index).substr(2) + ".png");
	
	unsigned int size = firstImage.width;
	unsigned int finalSize = (int) sqrt(size);

	Image toReturn = newImage(finalSize*size, finalSize*size);

	for (unsigned int x = 0; x < finalSize; x++) {
		for (unsigned int y = 0; y < finalSize; y++) {

			Image img = loadImage(folder + to_string(index).substr(2) + ".png");
			paste(img, toReturn, x*size, y*size);
			index++;
		}
	}

	lodepng::encode( (folder + "texture.png"), toReturn.img, toReturn.width, toReturn.height);

	return toReturn;
}

void makeVoxelTexture(int argc, char** argv) {

	if (argc != 3) {
		cerr << "ERROR; Parameters must be : 1) folder 2) nb of digits" << endl;
	}
	else {
		int nb_digits = atoi(argv[2]);
		string folder = argv[1]; folder += "/";
		Image tex = loadVoxelImages(folder, nb_digits);
		writeImage(tex, folder);
	}
}