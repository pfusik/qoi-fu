// Generated automatically with "cito". Do not edit.
#pragma once
#include <cstdint>
#include <memory>
class QOIDecoder;

class QOIDecoder
{
public:
	QOIDecoder();
	bool decode(uint8_t const * encoded, int encodedSize);
	int getWidth() const;
	int getHeight() const;
	int const * getPixels() const;
	bool getAlpha() const;
	int getColorspace() const;
private:
	int width;
	int height;
	std::shared_ptr<int[]> pixels;
	bool alpha;
	int colorspace;
};
