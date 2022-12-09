// Generated automatically with "cito". Do not edit.
#pragma once
#include <cstdint>
#include <memory>
class QOIEncoder;
class QOIDecoder;

/**
 * Encoder of the "Quite OK Image" (QOI) format.
 * Losslessly compresses an image to a byte array.
 */
class QOIEncoder
{
public:
	/**
	 * Constructs the encoder.
	 * The encoder can be used for several images, one after another.
	 */
	QOIEncoder();
	/**
	 * Determines if an image of given size can be encoded.
	 * @param width Image width in pixels.
	 * @param height Image height in pixels.
	 * @param alpha Whether the image has the alpha channel (transparency).
	 */
	static bool canEncode(int width, int height, bool alpha);
	/**
	 * Encodes the given image.
	 * Returns <code>true</code> if encoded successfully.
	 * @param width Image width in pixels.
	 * @param height Image height in pixels.
	 * @param pixels Pixels of the image, top-down, left-to-right.
	 * @param alpha <code>false</code> specifies that all pixels are opaque. High bytes of <code>pixels</code> elements are ignored then.
	 * @param linearColorspace Specifies the color space.
	 */
	bool encode(int width, int height, int const * pixels, bool alpha, bool linearColorspace);
	/**
	 * Returns the encoded file contents.
	 * This method can only be called after <code>Encode</code> returned <code>true</code>.
	 * The allocated array is usually larger than the encoded data.
	 * Call <code>GetEncodedSize</code> to retrieve the number of leading bytes that are significant.
	 */
	uint8_t const * getEncoded() const;
	/**
	 * Returns the encoded file length.
	 */
	int getEncodedSize() const;
public:
	static constexpr int headerSize = 14;
	static constexpr int paddingSize = 8;
private:
	std::shared_ptr<uint8_t[]> encoded;
	int encodedSize;
};

/**
 * Decoder of the "Quite OK Image" (QOI) format.
 */
class QOIDecoder
{
public:
	/**
	 * Constructs the decoder.
	 * The decoder can be used for several images, one after another.
	 */
	QOIDecoder();
	/**
	 * Decodes the given QOI file contents.
	 * Returns <code>true</code> if decoded successfully.
	 * @param encoded QOI file contents. Only the first <code>encodedSize</code> bytes are accessed.
	 * @param encodedSize QOI file length.
	 */
	bool decode(uint8_t const * encoded, int encodedSize);
	/**
	 * Returns the width of the decoded image in pixels.
	 */
	int getWidth() const;
	/**
	 * Returns the height of the decoded image in pixels.
	 */
	int getHeight() const;
	/**
	 * Returns the pixels of the decoded image, top-down, left-to-right.
	 * Each pixel is a 32-bit integer 0xAARRGGBB.
	 */
	int const * getPixels() const;
	/**
	 * Returns the information about the alpha channel from the file header.
	 */
	bool hasAlpha() const;
	/**
	 * Returns the color space information from the file header.
	 * <code>false</code> = sRGB with linear alpha channel.
	 * <code>true</code> = all channels linear.
	 */
	bool isLinearColorspace() const;
private:
	int width;
	int height;
	std::shared_ptr<int[]> pixels;
	bool alpha;
	bool linearColorspace;
};
