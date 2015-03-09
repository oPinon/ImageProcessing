#include "Tiler.h"

using namespace std;

void image2Tiles(const Image& img, uint tileSize, std::string mainFolder) {

	_mkdir(mainFolder.c_str());

	uint width = img.width; uint height = img.height;

	vector<Image> mipmaps(0);
	mipmaps.push_back(img);

	uint w = width; uint h = height;
	do {
		Image downScaled = downscaleBy2(mipmaps[mipmaps.size() - 1]);
		w = downScaled.width; h = downScaled.height;
		mipmaps.push_back(downScaled);
	} while (w > tileSize || h > tileSize);

	for (uint zoom = 0; zoom < mipmaps.size(); zoom++) {

		const Image& fullIm = mipmaps[mipmaps.size() - 1 - zoom];

		for (uint x0 = 0; x0 < fullIm.width; x0+=tileSize) {
			for (uint y0 = 0; y0 < fullIm.height; y0 += tileSize) {

				Image tile = newImage(tileSize, tileSize);

				for (uint x = 0; x < tileSize && x0 + x < fullIm.width; x++) {
					for (uint y = 0; y < tileSize && y0 + y < fullIm.height; y++) {
						for (uint c = 0; c < 4; c++) {
							tile.img[4*(x + y*tileSize)+c] = fullIm.img[4*(x0 + x + (y0 + y)*fullIm.width)+c];
						}
					}
				}

				std::string outFile = mainFolder;
				outFile += '/' + std::to_string(zoom);
				_mkdir(outFile.c_str());
				outFile += '/' + std::to_string(x0 / tileSize);
				_mkdir(outFile.c_str());
				outFile += '/' + std::to_string(y0 / tileSize);
				outFile += ".png";
				cout << outFile << endl;
				writeImage(tile, outFile);
			}
		}
	}
}

Image downscaleBy2(const Image& img) {

	uint width = img.width; uint height = img.height;
	uint newW = width / 2; uint newH = height / 2;

	Image toReturn = newImage(newW, newH);
	for (uint c = 0; c < 4; c++) {
		for (uint x = 0; x < newW; x++) {
			for (uint y = 0; y < newH; y++) {
				unsigned char p00 = img.img[4 * (2 * x + width * 2 * y) + c];
				unsigned char p01 = img.img[4 * (2 * x + width*(2 * y + 1)) + c];
				unsigned char p10 = img.img[4 * (2 * x + 1 + width * 2 * y) + c];
				unsigned char p11 = img.img[4 * (2 * x + 1 + width*(2 * y + 1)) + c];
				toReturn.img[4 * (x + y*newW)+c] = (p00 + p01 + p10 + p11) / 4;
			}
		}
	}

	return toReturn;
}