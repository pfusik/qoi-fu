// Generated automatically with "cito". Do not edit.
#include <algorithm>
#include <array>
#include "QOI.hpp"
QOIEncoder::QOIEncoder()
{
}

bool QOIEncoder::canEncode(int width, int height, bool alpha)
{
	return width > 0 && height > 0 && height <= 2147483625 / width / (alpha ? 5 : 4);
}

bool QOIEncoder::encode(int width, int height, int const * pixels, bool alpha, bool linearColorspace)
{
	if (pixels == nullptr || !canEncode(width, height, alpha))
		return false;
	int pixelsSize = width * height;
	std::shared_ptr<uint8_t[]> encoded = std::make_shared<uint8_t[]>(14 + pixelsSize * (alpha ? 5 : 4) + 8);
	encoded[0] = 'q';
	encoded[1] = 'o';
	encoded[2] = 'i';
	encoded[3] = 'f';
	encoded[4] = static_cast<uint8_t>(width >> 24);
	encoded[5] = static_cast<uint8_t>(width >> 16);
	encoded[6] = static_cast<uint8_t>(width >> 8);
	encoded[7] = static_cast<uint8_t>(width);
	encoded[8] = static_cast<uint8_t>(height >> 24);
	encoded[9] = static_cast<uint8_t>(height >> 16);
	encoded[10] = static_cast<uint8_t>(height >> 8);
	encoded[11] = static_cast<uint8_t>(height);
	encoded[12] = static_cast<uint8_t>(alpha ? 4 : 3);
	encoded[13] = static_cast<uint8_t>(linearColorspace ? 1 : 0);
	std::array<int, 64> index {};
	int encodedOffset = 14;
	int lastPixel = -16777216;
	int run = 0;
	for (int pixelsOffset = 0; pixelsOffset < pixelsSize;) {
		int pixel = pixels[pixelsOffset++];
		if (!alpha)
			pixel |= -16777216;
		if (pixel == lastPixel) {
			if (++run == 62 || pixelsOffset >= pixelsSize) {
				encoded[encodedOffset++] = static_cast<uint8_t>(191 + run);
				run = 0;
			}
		}
		else {
			if (run > 0) {
				encoded[encodedOffset++] = static_cast<uint8_t>(191 + run);
				run = 0;
			}
			int indexOffset = ((pixel >> 16) * 3 + (pixel >> 8) * 5 + (pixel & 63) * 7 + (pixel >> 24) * 11) & 63;
			if (pixel == index[indexOffset])
				encoded[encodedOffset++] = static_cast<uint8_t>(indexOffset);
			else {
				index[indexOffset] = pixel;
				int r = pixel >> 16 & 255;
				int g = pixel >> 8 & 255;
				int b = pixel & 255;
				int a = pixel >> 24 & 255;
				if ((pixel ^ lastPixel) >> 24 != 0) {
					encoded[encodedOffset] = 255;
					encoded[encodedOffset + 1] = static_cast<uint8_t>(r);
					encoded[encodedOffset + 2] = static_cast<uint8_t>(g);
					encoded[encodedOffset + 3] = static_cast<uint8_t>(b);
					encoded[encodedOffset + 4] = static_cast<uint8_t>(a);
					encodedOffset += 5;
				}
				else {
					int dr = r - (lastPixel >> 16 & 255);
					int dg = g - (lastPixel >> 8 & 255);
					int db = b - (lastPixel & 255);
					if (dr >= -2 && dr <= 1 && dg >= -2 && dg <= 1 && db >= -2 && db <= 1)
						encoded[encodedOffset++] = static_cast<uint8_t>(106 + (dr << 4) + (dg << 2) + db);
					else {
						dr -= dg;
						db -= dg;
						if (dr >= -8 && dr <= 7 && dg >= -32 && dg <= 31 && db >= -8 && db <= 7) {
							encoded[encodedOffset] = static_cast<uint8_t>(160 + dg);
							encoded[encodedOffset + 1] = static_cast<uint8_t>(136 + (dr << 4) + db);
							encodedOffset += 2;
						}
						else {
							encoded[encodedOffset] = 254;
							encoded[encodedOffset + 1] = static_cast<uint8_t>(r);
							encoded[encodedOffset + 2] = static_cast<uint8_t>(g);
							encoded[encodedOffset + 3] = static_cast<uint8_t>(b);
							encodedOffset += 4;
						}
					}
				}
			}
			lastPixel = pixel;
		}
	}
	std::fill_n(encoded.get() + encodedOffset, 7, 0);
	encoded[encodedOffset + 8 - 1] = 1;
	this->encoded = encoded;
	this->encodedSize = encodedOffset + 8;
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
QOIDecoder::QOIDecoder()
{
}

bool QOIDecoder::decode(uint8_t const * encoded, int encodedSize)
{
	if (encoded == nullptr || encodedSize < 23 || encoded[0] != 'q' || encoded[1] != 'o' || encoded[2] != 'i' || encoded[3] != 'f')
		return false;
	int width = encoded[4] << 24 | encoded[5] << 16 | encoded[6] << 8 | encoded[7];
	int height = encoded[8] << 24 | encoded[9] << 16 | encoded[10] << 8 | encoded[11];
	if (width <= 0 || height <= 0 || height > 2147483647 / width)
		return false;
	switch (encoded[12]) {
	case 3:
		this->alpha = false;
		break;
	case 4:
		this->alpha = true;
		break;
	default:
		return false;
	}
	switch (encoded[13]) {
	case 0:
		this->linearColorspace = false;
		break;
	case 1:
		this->linearColorspace = true;
		break;
	default:
		return false;
	}
	int pixelsSize = width * height;
	std::shared_ptr<int[]> pixels = std::make_shared<int[]>(pixelsSize);
	encodedSize -= 8;
	int encodedOffset = 14;
	std::array<int, 64> index {};
	int pixel = -16777216;
	for (int pixelsOffset = 0; pixelsOffset < pixelsSize;) {
		if (encodedOffset >= encodedSize)
			return false;
		int e = encoded[encodedOffset++];
		switch (e >> 6) {
		case 0:
			pixels[pixelsOffset++] = pixel = index[e];
			continue;
		case 1:
			pixel = (pixel & -16777216) | ((pixel + (((e >> 4) - 4 - 2) << 16)) & 16711680) | ((pixel + (((e >> 2 & 3) - 2) << 8)) & 65280) | ((pixel + (e & 3) - 2) & 255);
			break;
		case 2:
			e -= 160;
			{
				int rb = encoded[encodedOffset++];
				pixel = (pixel & -16777216) | ((pixel + ((e + (rb >> 4) - 8) << 16)) & 16711680) | ((pixel + (e << 8)) & 65280) | ((pixel + e + (rb & 15) - 8) & 255);
				break;
			}
		default:
			if (e < 254) {
				e -= 191;
				if (pixelsOffset + e > pixelsSize)
					return false;
				std::fill_n(pixels.get() + pixelsOffset, e, pixel);
				pixelsOffset += e;
				continue;
			}
			if (e == 254) {
				pixel = (pixel & -16777216) | encoded[encodedOffset] << 16 | encoded[encodedOffset + 1] << 8 | encoded[encodedOffset + 2];
				encodedOffset += 3;
			}
			else {
				pixel = encoded[encodedOffset + 3] << 24 | encoded[encodedOffset] << 16 | encoded[encodedOffset + 1] << 8 | encoded[encodedOffset + 2];
				encodedOffset += 4;
			}
			break;
		}
		pixels[pixelsOffset++] = index[((pixel >> 16) * 3 + (pixel >> 8) * 5 + (pixel & 63) * 7 + (pixel >> 24) * 11) & 63] = pixel;
	}
	if (encodedOffset != encodedSize)
		return false;
	this->width = width;
	this->height = height;
	this->pixels = pixels;
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

bool QOIDecoder::hasAlpha() const
{
	return this->alpha;
}

bool QOIDecoder::isLinearColorspace() const
{
	return this->linearColorspace;
}
