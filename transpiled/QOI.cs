// Generated automatically with "fut". Do not edit.
using System;

/// <summary>Encoder of the "Quite OK Image" (QOI) format.</summary>
/// <remarks>Losslessly compresses an image to a byte array.</remarks>
public class QOIEncoder
{
	/// <summary>Constructs the encoder.</summary>
	/// <remarks>The encoder can be used for several images, one after another.</remarks>
	public QOIEncoder()
	{
	}

	internal const int HeaderSize = 14;

	internal const int PaddingSize = 8;

	byte[] Encoded;

	int EncodedSize;

	/// <summary>Determines if an image of given size can be encoded.</summary>
	/// <param name="width">Image width in pixels.</param>
	/// <param name="height">Image height in pixels.</param>
	/// <param name="alpha">Whether the image has the alpha channel (transparency).</param>
	public static bool CanEncode(int width, int height, bool alpha) => width > 0 && height > 0 && height <= 2147483625 / width / (alpha ? 5 : 4);

	/// <summary>Encodes the given image.</summary>
	/// <remarks>Returns <see langword="true" /> if encoded successfully.</remarks>
	/// <param name="width">Image width in pixels.</param>
	/// <param name="height">Image height in pixels.</param>
	/// <param name="pixels">Pixels of the image, top-down, left-to-right.</param>
	/// <param name="alpha"><see langword="false" /> specifies that all pixels are opaque. High bytes of <c>pixels</c> elements are ignored then.</param>
	/// <param name="linearColorspace">Specifies the color space.</param>
	public bool Encode(int width, int height, int[] pixels, bool alpha, bool linearColorspace)
	{
		if (!CanEncode(width, height, alpha))
			return false;
		int pixelsSize = width * height;
		byte[] encoded = new byte[14 + pixelsSize * (alpha ? 5 : 4) + 8];
		encoded[0] = (byte) 'q';
		encoded[1] = (byte) 'o';
		encoded[2] = (byte) 'i';
		encoded[3] = (byte) 'f';
		encoded[4] = (byte) (width >> 24);
		encoded[5] = (byte) (width >> 16);
		encoded[6] = (byte) (width >> 8);
		encoded[7] = (byte) width;
		encoded[8] = (byte) (height >> 24);
		encoded[9] = (byte) (height >> 16);
		encoded[10] = (byte) (height >> 8);
		encoded[11] = (byte) height;
		encoded[12] = (byte) (alpha ? 4 : 3);
		encoded[13] = (byte) (linearColorspace ? 1 : 0);
		int[] index = new int[64];
		int encodedOffset = 14;
		int lastPixel = -16777216;
		int run = 0;
		for (int pixelsOffset = 0; pixelsOffset < pixelsSize;) {
			int pixel = pixels[pixelsOffset++];
			if (!alpha)
				pixel |= -16777216;
			if (pixel == lastPixel) {
				if (++run == 62 || pixelsOffset >= pixelsSize) {
					encoded[encodedOffset++] = (byte) (191 + run);
					run = 0;
				}
			}
			else {
				if (run > 0) {
					encoded[encodedOffset++] = (byte) (191 + run);
					run = 0;
				}
				int indexOffset = ((pixel >> 16) * 3 + (pixel >> 8) * 5 + (pixel & 63) * 7 + (pixel >> 24) * 11) & 63;
				if (pixel == index[indexOffset])
					encoded[encodedOffset++] = (byte) indexOffset;
				else {
					index[indexOffset] = pixel;
					int r = pixel >> 16 & 255;
					int g = pixel >> 8 & 255;
					int b = pixel & 255;
					if ((pixel ^ lastPixel) >> 24 != 0) {
						encoded[encodedOffset] = 255;
						encoded[encodedOffset + 1] = (byte) r;
						encoded[encodedOffset + 2] = (byte) g;
						encoded[encodedOffset + 3] = (byte) b;
						encoded[encodedOffset + 4] = (byte) (pixel >> 24);
						encodedOffset += 5;
					}
					else {
						int dr = r - (lastPixel >> 16 & 255);
						int dg = g - (lastPixel >> 8 & 255);
						int db = b - (lastPixel & 255);
						if (dr >= -2 && dr <= 1 && dg >= -2 && dg <= 1 && db >= -2 && db <= 1)
							encoded[encodedOffset++] = (byte) (106 + (dr << 4) + (dg << 2) + db);
						else {
							dr -= dg;
							db -= dg;
							if (dr >= -8 && dr <= 7 && dg >= -32 && dg <= 31 && db >= -8 && db <= 7) {
								encoded[encodedOffset] = (byte) (160 + dg);
								encoded[encodedOffset + 1] = (byte) (136 + (dr << 4) + db);
								encodedOffset += 2;
							}
							else {
								encoded[encodedOffset] = 254;
								encoded[encodedOffset + 1] = (byte) r;
								encoded[encodedOffset + 2] = (byte) g;
								encoded[encodedOffset + 3] = (byte) b;
								encodedOffset += 4;
							}
						}
					}
				}
				lastPixel = pixel;
			}
		}
		Array.Clear(encoded, encodedOffset, 7);
		encoded[encodedOffset + 8 - 1] = 1;
		this.Encoded = encoded;
		this.EncodedSize = encodedOffset + 8;
		return true;
	}

	/// <summary>Returns the encoded file contents.</summary>
	/// <remarks>This method can only be called after <c>Encode</c> returned <see langword="true" />.
	/// The allocated array is usually larger than the encoded data.
	/// Call <c>GetEncodedSize</c> to retrieve the number of leading bytes that are significant.</remarks>
	public byte[] GetEncoded() => this.Encoded;

	/// <summary>Returns the encoded file length.</summary>
	public int GetEncodedSize() => this.EncodedSize;
}

/// <summary>Decoder of the "Quite OK Image" (QOI) format.</summary>
public class QOIDecoder
{
	/// <summary>Constructs the decoder.</summary>
	/// <remarks>The decoder can be used for several images, one after another.</remarks>
	public QOIDecoder()
	{
	}

	int Width;

	int Height;

	int[] Pixels;

	bool Alpha;

	bool LinearColorspace;

	/// <summary>Decodes the given QOI file contents.</summary>
	/// <remarks>Returns <see langword="true" /> if decoded successfully.</remarks>
	/// <param name="encoded">QOI file contents. Only the first <c>encodedSize</c> bytes are accessed.</param>
	/// <param name="encodedSize">QOI file length.</param>
	public bool Decode(byte[] encoded, int encodedSize)
	{
		if (encodedSize < 23 || encoded[0] != 'q' || encoded[1] != 'o' || encoded[2] != 'i' || encoded[3] != 'f')
			return false;
		int width = encoded[4] << 24 | encoded[5] << 16 | encoded[6] << 8 | encoded[7];
		int height = encoded[8] << 24 | encoded[9] << 16 | encoded[10] << 8 | encoded[11];
		if (width <= 0 || height <= 0 || height > 2147483647 / width)
			return false;
		switch (encoded[12]) {
		case 3:
			this.Alpha = false;
			break;
		case 4:
			this.Alpha = true;
			break;
		default:
			return false;
		}
		switch (encoded[13]) {
		case 0:
			this.LinearColorspace = false;
			break;
		case 1:
			this.LinearColorspace = true;
			break;
		default:
			return false;
		}
		int pixelsSize = width * height;
		int[] pixels = new int[pixelsSize];
		encodedSize -= 8;
		int encodedOffset = 14;
		int[] index = new int[64];
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
					Array.Fill(pixels, pixel, pixelsOffset, e);
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
		this.Width = width;
		this.Height = height;
		this.Pixels = pixels;
		return true;
	}

	/// <summary>Returns the width of the decoded image in pixels.</summary>
	public int GetWidth() => this.Width;

	/// <summary>Returns the height of the decoded image in pixels.</summary>
	public int GetHeight() => this.Height;

	/// <summary>Returns the pixels of the decoded image, top-down, left-to-right.</summary>
	/// <remarks>Each pixel is a 32-bit integer 0xAARRGGBB.</remarks>
	public int[] GetPixels() => this.Pixels;

	/// <summary>Returns the information about the alpha channel from the file header.</summary>
	public bool HasAlpha() => this.Alpha;

	/// <summary>Returns the color space information from the file header.</summary>
	/// <remarks><see langword="false" /> = sRGB with linear alpha channel.
	/// <see langword="true" /> = all channels linear.</remarks>
	public bool IsLinearColorspace() => this.LinearColorspace;
}
