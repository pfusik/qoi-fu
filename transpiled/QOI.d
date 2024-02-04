// Generated automatically with "fut". Do not edit.
import std.algorithm;

/// Encoder of the "Quite OK Image" (QOI) format.
/// Losslessly compresses an image to a byte array.
class QOIEncoder
{
	/// Constructs the encoder.
	/// The encoder can be used for several images, one after another.
	this()
	{
	}
	static immutable int headerSize = 14;
	static immutable int paddingSize = 8;

	private ubyte[] encoded;

	private int encodedSize;

	/// Determines if an image of given size can be encoded.
	/// Params:
	/// width = Image width in pixels.
	/// height = Image height in pixels.
	/// alpha = Whether the image has the alpha channel (transparency).
	static bool canEncode(int width, int height, bool alpha) => width > 0 && height > 0 && height <= 2147483625 / width / (alpha ? cast(ubyte)(5) : cast(ubyte)(4));

	/// Encodes the given image.
	/// Returns `true` if encoded successfully.
	/// Params:
	/// width = Image width in pixels.
	/// height = Image height in pixels.
	/// pixels = Pixels of the image, top-down, left-to-right.
	/// alpha = `false` specifies that all pixels are opaque. High bytes of `pixels` elements are ignored then.
	/// linearColorspace = Specifies the color space.
	bool encode(int width, int height, const(int)[] pixels, bool alpha, bool linearColorspace)
	{
		if (!canEncode(width, height, alpha))
			return false;
		int pixelsSize = width * height;
		ubyte[] encoded = new ubyte[14 + pixelsSize * (alpha ? cast(ubyte)(5) : cast(ubyte)(4)) + 8];
		encoded[0] = 'q';
		encoded[1] = 'o';
		encoded[2] = 'i';
		encoded[3] = 'f';
		encoded[4] = cast(ubyte)(width >> 24);
		encoded[5] = cast(ubyte)(width >> 16);
		encoded[6] = cast(ubyte)(width >> 8);
		encoded[7] = cast(ubyte)(width);
		encoded[8] = cast(ubyte)(height >> 24);
		encoded[9] = cast(ubyte)(height >> 16);
		encoded[10] = cast(ubyte)(height >> 8);
		encoded[11] = cast(ubyte)(height);
		encoded[12] = cast(ubyte)(alpha ? cast(ubyte)(4) : cast(ubyte)(3));
		encoded[13] = cast(ubyte)(linearColorspace ? cast(ubyte)(1) : cast(ubyte)(0));
		int[64] index;
		int encodedOffset = 14;
		int lastPixel = -16777216;
		int run = 0;
		for (int pixelsOffset = 0; pixelsOffset < pixelsSize;) {
			int pixel = pixels[pixelsOffset++];
			if (!alpha)
				pixel |= -16777216;
			if (pixel == lastPixel) {
				if (++run == 62 || pixelsOffset >= pixelsSize) {
					encoded[encodedOffset++] = cast(ubyte)(191 + run);
					run = 0;
				}
			}
			else {
				if (run > 0) {
					encoded[encodedOffset++] = cast(ubyte)(191 + run);
					run = 0;
				}
				int indexOffset = ((pixel >> 16) * 3 + (pixel >> 8) * 5 + (pixel & 63) * 7 + (pixel >> 24) * 11) & 63;
				if (pixel == index[indexOffset])
					encoded[encodedOffset++] = cast(ubyte)(indexOffset);
				else {
					index[indexOffset] = pixel;
					int r = pixel >> 16 & 255;
					int g = pixel >> 8 & 255;
					int b = pixel & 255;
					if ((pixel ^ lastPixel) >> 24 != 0) {
						encoded[encodedOffset] = 255;
						encoded[encodedOffset + 1] = cast(ubyte)(r);
						encoded[encodedOffset + 2] = cast(ubyte)(g);
						encoded[encodedOffset + 3] = cast(ubyte)(b);
						encoded[encodedOffset + 4] = cast(ubyte)(pixel >> 24);
						encodedOffset += 5;
					}
					else {
						int dr = (((r - (lastPixel >> 16)) & 255) ^ 128) - 128;
						int dg = (((g - (lastPixel >> 8)) & 255) ^ 128) - 128;
						int db = (((b - lastPixel) & 255) ^ 128) - 128;
						if (dr >= -2 && dr <= 1 && dg >= -2 && dg <= 1 && db >= -2 && db <= 1)
							encoded[encodedOffset++] = cast(ubyte)(106 + (dr << 4) + (dg << 2) + db);
						else {
							dr -= dg;
							db -= dg;
							if (dr >= -8 && dr <= 7 && dg >= -32 && dg <= 31 && db >= -8 && db <= 7) {
								encoded[encodedOffset] = cast(ubyte)(160 + dg);
								encoded[encodedOffset + 1] = cast(ubyte)(136 + (dr << 4) + db);
								encodedOffset += 2;
							}
							else {
								encoded[encodedOffset] = 254;
								encoded[encodedOffset + 1] = cast(ubyte)(r);
								encoded[encodedOffset + 2] = cast(ubyte)(g);
								encoded[encodedOffset + 3] = cast(ubyte)(b);
								encodedOffset += 4;
							}
						}
					}
				}
				lastPixel = pixel;
			}
		}
		encoded[encodedOffset .. $][0 .. 7].fill(cast(ubyte)(0));
		encoded[encodedOffset + 8 - 1] = 1;
		this.encoded = encoded;
		this.encodedSize = encodedOffset + 8;
		return true;
	}

	/// Returns the encoded file contents.
	/// This method can only be called after `Encode` returned `true`.
	/// The allocated array is usually larger than the encoded data.
	/// Call `GetEncodedSize` to retrieve the number of leading bytes that are significant.
	const(ubyte)[] getEncoded() => this.encoded;

	/// Returns the encoded file length.
	int getEncodedSize() => this.encodedSize;
}

/// Decoder of the "Quite OK Image" (QOI) format.
class QOIDecoder
{
	/// Constructs the decoder.
	/// The decoder can be used for several images, one after another.
	this()
	{
	}

	private int width;

	private int height;

	private int[] pixels;

	private bool alpha;

	private bool linearColorspace;

	/// Decodes the given QOI file contents.
	/// Returns `true` if decoded successfully.
	/// Params:
	/// encoded = QOI file contents. Only the first `encodedSize` bytes are accessed.
	/// encodedSize = QOI file length.
	bool decode(const(ubyte)[] encoded, int encodedSize)
	{
		if (encodedSize < 23 || encoded[0] != 'q' || encoded[1] != 'o' || encoded[2] != 'i' || encoded[3] != 'f')
			return false;
		int width = encoded[4] << 24 | encoded[5] << 16 | encoded[6] << 8 | encoded[7];
		int height = encoded[8] << 24 | encoded[9] << 16 | encoded[10] << 8 | encoded[11];
		if (width <= 0 || height <= 0 || height > 2147483647 / width)
			return false;
		switch (encoded[12]) {
		case 3:
			this.alpha = false;
			break;
		case 4:
			this.alpha = true;
			break;
		default:
			return false;
		}
		switch (encoded[13]) {
		case 0:
			this.linearColorspace = false;
			break;
		case 1:
			this.linearColorspace = true;
			break;
		default:
			return false;
		}
		int pixelsSize = width * height;
		int[] pixels = new int[pixelsSize];
		encodedSize -= 8;
		int encodedOffset = 14;
		int[64] index;
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
				int rb = encoded[encodedOffset++];
				pixel = (pixel & -16777216) | ((pixel + ((e + (rb >> 4) - 8) << 16)) & 16711680) | ((pixel + (e << 8)) & 65280) | ((pixel + e + (rb & 15) - 8) & 255);
				break;
			default:
				if (e < 254) {
					e -= 191;
					if (pixelsOffset + e > pixelsSize)
						return false;
					if (pixelsOffset == 0)
						index[53] = pixel;
					pixels[pixelsOffset .. $][0 .. e].fill(pixel);
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
		this.width = width;
		this.height = height;
		this.pixels = pixels;
		return true;
	}

	/// Returns the width of the decoded image in pixels.
	int getWidth() => this.width;

	/// Returns the height of the decoded image in pixels.
	int getHeight() => this.height;

	/// Returns the pixels of the decoded image, top-down, left-to-right.
	/// Each pixel is a 32-bit integer 0xAARRGGBB.
	const(int)[] getPixels() => this.pixels;

	/// Returns the information about the alpha channel from the file header.
	bool hasAlpha() => this.alpha;

	/// Returns the color space information from the file header.
	/// `false` = sRGB with linear alpha channel.
	/// `true` = all channels linear.
	bool isLinearColorspace() => this.linearColorspace;
}
