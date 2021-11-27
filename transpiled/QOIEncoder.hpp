// Generated automatically with "cito". Do not edit.
#pragma once
#include <cstdint>
#include <memory>
class QOIEncoder;

class QOIEncoder
{
public:
	QOIEncoder();
	bool encode(int width, int height, int const * pixels, bool alpha);
	uint8_t const * getEncoded() const;
	int getEncodedSize() const;
private:
	static constexpr int headerSize = 12;
	std::shared_ptr<uint8_t[]> encoded;
	int encodedSize;
};
