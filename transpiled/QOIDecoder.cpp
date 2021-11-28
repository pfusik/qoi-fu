// Generated automatically with "cito". Do not edit.
#include <algorithm>
#include <array>
#include "QOIDecoder.hpp"
QOIDecoder::QOIDecoder()
{
}

bool QOIDecoder::decode(uint8_t const * encoded, int encodedSize)
{
	if (encoded == nullptr || encodedSize < 19 || encoded[0] != 113 || encoded[1] != 111 || encoded[2] != 105 || encoded[3] != 102)
		return false;
	int width = encoded[4] << 24 | encoded[5] << 16 | encoded[6] << 8 | encoded[7];
	int height = encoded[8] << 24 | encoded[9] << 16 | encoded[10] << 8 | encoded[11];
	if (width <= 0 || height <= 0 || height > 2147483647 / width)
		return false;
	int pixelsSize = width * height;
	std::shared_ptr<int[]> pixels = std::make_shared<int[]>(pixelsSize);
	encodedSize -= 4;
	int encodedOffset = 14;
	std::array<int, 64> index {};
	int pixel = -16777216;
	for (int pixelsOffset = 0; pixelsOffset < pixelsSize;) {
		if (encodedOffset >= encodedSize)
			return false;
		int e = encoded[encodedOffset++];
		if (e < 128) {
			if (e < 64)
				pixels[pixelsOffset++] = pixel = index[e];
			else {
				int run = 1 + (e & 31);
				if (e >= 96)
					run += 32 + encoded[encodedOffset++];
				if (pixelsOffset + run > pixelsSize)
					return false;
				std::fill_n(pixels.get() + pixelsOffset, run, pixel);
				pixelsOffset += run;
			}
			continue;
		}
		else if (e < 224) {
			if (e < 192)
				pixel = (pixel & -16777216) | ((pixel + (((e >> 4) - 8 - 2) << 16)) & 16711680) | ((pixel + (((e >> 2 & 3) - 2) << 8)) & 65280) | ((pixel + (e & 3) - 2) & 255);
			else {
				int d = encoded[encodedOffset++];
				pixel = (pixel & -16777216) | ((pixel + ((e - 192 - 16) << 16)) & 16711680) | ((pixel + (((d >> 4) - 8) << 8)) & 65280) | ((pixel + (d & 15) - 8) & 255);
			}
		}
		else if (e < 240) {
			e = e << 16 | encoded[encodedOffset] << 8 | encoded[encodedOffset + 1];
			encodedOffset += 2;
			pixel = ((pixel + (((e & 31) - 16) << 24)) & -16777216) | ((pixel + (((e >> 15) - 448 - 16) << 16)) & 16711680) | ((pixel + (((e >> 10 & 31) - 16) << 8)) & 65280) | ((pixel + (e >> 5 & 31) - 16) & 255);
		}
		else {
			if ((e & 8) != 0)
				pixel = (pixel & -16711681) | encoded[encodedOffset++] << 16;
			if ((e & 4) != 0)
				pixel = (pixel & -65281) | encoded[encodedOffset++] << 8;
			if ((e & 2) != 0)
				pixel = (pixel & -256) | encoded[encodedOffset++];
			if ((e & 1) != 0)
				pixel = (pixel & 16777215) | encoded[encodedOffset++] << 24;
		}
		pixels[pixelsOffset++] = index[(pixel >> 24 ^ pixel >> 16 ^ pixel >> 8 ^ pixel) & 63] = pixel;
	}
	if (encodedOffset != encodedSize)
		return false;
	this->width = width;
	this->height = height;
	this->pixels = pixels;
	this->alpha = encoded[12] == 4;
	this->colorspace = encoded[13];
	return true;
}

int QOIDecoder::getWidth() const
{
	return this->width;
}

int QOIDecoder::getHeight() const
{
	return this->height;
}

int const * QOIDecoder::getPixels() const
{
	return this->pixels.get();
}

bool QOIDecoder::getAlpha() const
{
	return this->alpha;
}

int QOIDecoder::getColorspace() const
{
	return this->colorspace;
}
