// Generated automatically with "cito". Do not edit.
#pragma once
#include <cstdint>
#include <memory>
class QOIEncoder;

class QOIEncoder
{
public:
	QOIEncoder();
	static constexpr int colorspaceSrgb = 0;
	static constexpr int colorspaceSrgbLinearAlpha = 1;
	static constexpr int colorspaceLinear = 15;
	bool encode(int width, int height, int const * pixels, bool alpha, int colorspace);
	uint8_t const * getEncoded() const;
	int getEncodedSize() const;
private:
	static constexpr int headerSize = 14;
	std::shared_ptr<uint8_t[]> encoded;
	int encodedSize;
};
