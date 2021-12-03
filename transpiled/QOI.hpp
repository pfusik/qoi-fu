// Generated automatically with "cito". Do not edit.
#pragma once
#include <cstdint>
#include <memory>
class QOIColorspace;
class QOIEncoder;
class QOIDecoder;

class QOIColorspace
{
public:
	static constexpr int srgb = 0;
	static constexpr int srgbLinearAlpha = 1;
	static constexpr int linear = 15;
private:
	QOIColorspace() = delete;
};

class QOIEncoder
{
public:
	QOIEncoder();
	static bool canEncode(int width, int height, bool alpha);
	bool encode(int width, int height, int const * pixels, bool alpha, int colorspace);
	uint8_t const * getEncoded() const;
	int getEncodedSize() const;
public:
	static constexpr int headerSize = 14;
	static constexpr int paddingSize = 4;
private:
	std::shared_ptr<uint8_t[]> encoded;
	int encodedSize;
};

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
