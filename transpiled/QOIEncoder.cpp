// Generated automatically with "cito". Do not edit.
#include <algorithm>
#include <array>
#include "QOIEncoder.hpp"
QOIEncoder::QOIEncoder()
{
}

bool QOIEncoder::encode(int width, int height, int const * pixels, bool alpha, int colorspace)
{
	if (width <= 0 || height <= 0 || height > 429496725 / width || pixels == nullptr)
		return false;
	int pixelsSize = width * height;
	std::shared_ptr<uint8_t[]> encoded = std::make_shared<uint8_t[]>(14 + pixelsSize * (alpha ? 5 : 4) + 4);
	encoded[0] = 113;
	encoded[1] = 111;
	encoded[2] = 105;
	encoded[3] = 102;
	encoded[4] = static_cast<uint8_t>(width >> 24);
	encoded[5] = static_cast<uint8_t>(width >> 16);
	encoded[6] = static_cast<uint8_t>(width >> 8);
	encoded[7] = static_cast<uint8_t>(width);
	encoded[8] = static_cast<uint8_t>(height >> 24);
	encoded[9] = static_cast<uint8_t>(height >> 16);
	encoded[10] = static_cast<uint8_t>(height >> 8);
	encoded[11] = static_cast<uint8_t>(height);
	encoded[12] = static_cast<uint8_t>(alpha ? 4 : 3);
	encoded[13] = static_cast<uint8_t>(colorspace);
	std::array<int, 64> index {};
	int encodedOffset = 14;
	int lastPixel = -16777216;
	int run = 0;
	for (int pixelsOffset = 0; pixelsOffset < pixelsSize;) {
		int pixel = pixels[pixelsOffset++];
		if (!alpha)
			pixel |= -16777216;
		if (pixel == lastPixel)
			run++;
		if (run > 0 && (pixel != lastPixel || run == 8224 || pixelsOffset >= pixelsSize)) {
			if (run <= 32)
				encoded[encodedOffset++] = static_cast<uint8_t>(64 + run - 1);
			else {
				run -= 33;
				encoded[encodedOffset++] = static_cast<uint8_t>(96 + (run >> 8));
				encoded[encodedOffset++] = static_cast<uint8_t>(run);
			}
			run = 0;
		}
		if (pixel != lastPixel) {
			int indexOffset = (pixel >> 24 ^ pixel >> 16 ^ pixel >> 8 ^ pixel) & 63;
			if (pixel == index[indexOffset])
				encoded[encodedOffset++] = static_cast<uint8_t>(indexOffset);
			else {
				index[indexOffset] = pixel;
				int r = pixel >> 16 & 255;
				int g = pixel >> 8 & 255;
				int b = pixel & 255;
				int a = pixel >> 24 & 255;
				int dr = r - (lastPixel >> 16 & 255);
				int dg = g - (lastPixel >> 8 & 255);
				int db = b - (lastPixel & 255);
				int da = a - (lastPixel >> 24 & 255);
				if (dr >= -16 && dr <= 15 && dg >= -16 && dg <= 15 && db >= -16 && db <= 15 && da >= -16 && da <= 15) {
					if (da == 0 && dr >= -2 && dr <= 1 && dg >= -2 && dg <= 1 && db >= -2 && db <= 1)
						encoded[encodedOffset++] = static_cast<uint8_t>(170 + (dr << 4) + (dg << 2) + db);
					else if (da == 0 && dg >= -8 && dg <= 7 && db >= -8 && db <= 7) {
						encoded[encodedOffset++] = static_cast<uint8_t>(208 + dr);
						encoded[encodedOffset++] = static_cast<uint8_t>(136 + (dg << 4) + db);
					}
					else {
						dr += 16;
						encoded[encodedOffset++] = static_cast<uint8_t>(224 + (dr >> 1));
						db += 16;
						encoded[encodedOffset++] = static_cast<uint8_t>(((dr & 1) << 7) + ((dg + 16) << 2) + (db >> 3));
						encoded[encodedOffset++] = static_cast<uint8_t>(((db & 7) << 5) + da + 16);
					}
				}
				else {
					encoded[encodedOffset++] = static_cast<uint8_t>(240 | (dr != 0 ? 8 : 0) | (dg != 0 ? 4 : 0) | (db != 0 ? 2 : 0) | (da != 0 ? 1 : 0));
					if (dr != 0)
						encoded[encodedOffset++] = static_cast<uint8_t>(r);
					if (dg != 0)
						encoded[encodedOffset++] = static_cast<uint8_t>(g);
					if (db != 0)
						encoded[encodedOffset++] = static_cast<uint8_t>(b);
					if (da != 0)
						encoded[encodedOffset++] = static_cast<uint8_t>(a);
				}
			}
			lastPixel = pixel;
		}
	}
	std::fill_n(encoded.get() + encodedOffset, 4, 0);
	this->encoded = encoded;
	this->encodedSize = encodedOffset + 4;
	return true;
}

uint8_t const * QOIEncoder::getEncoded() const
{
	return this->encoded.get();
}

int QOIEncoder::getEncodedSize() const
{
	return this->encodedSize;
}
