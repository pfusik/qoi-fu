// Generated automatically with "cito". Do not edit.
using System;

public class QOIEncoder
{
	public QOIEncoder()
	{
	}

	public const int ColorspaceSrgb = 0;

	public const int ColorspaceSrgbLinearAlpha = 1;

	public const int ColorspaceLinear = 15;

	const int HeaderSize = 14;

	byte[] Encoded;

	int EncodedSize;

	public bool Encode(int width, int height, int[] pixels, bool alpha, int colorspace)
	{
		if (width <= 0 || height <= 0 || height > 429496725 / width || pixels == null)
			return false;
		int pixelsSize = width * height;
		byte[] encoded = new byte[14 + pixelsSize * (alpha ? 5 : 4) + 4];
		encoded[0] = 113;
		encoded[1] = 111;
		encoded[2] = 105;
		encoded[3] = 102;
		encoded[4] = (byte) (width >> 24);
		encoded[5] = (byte) (width >> 16);
		encoded[6] = (byte) (width >> 8);
		encoded[7] = (byte) width;
		encoded[8] = (byte) (height >> 24);
		encoded[9] = (byte) (height >> 16);
		encoded[10] = (byte) (height >> 8);
		encoded[11] = (byte) height;
		encoded[12] = (byte) (alpha ? 4 : 3);
		encoded[13] = (byte) colorspace;
		int[] index = new int[64];
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
					encoded[encodedOffset++] = (byte) (64 + run - 1);
				else {
					run -= 33;
					encoded[encodedOffset++] = (byte) (96 + (run >> 8));
					encoded[encodedOffset++] = (byte) run;
				}
				run = 0;
			}
			if (pixel != lastPixel) {
				int indexOffset = (pixel >> 24 ^ pixel >> 16 ^ pixel >> 8 ^ pixel) & 63;
				if (pixel == index[indexOffset])
					encoded[encodedOffset++] = (byte) indexOffset;
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
							encoded[encodedOffset++] = (byte) (170 + (dr << 4) + (dg << 2) + db);
						else if (da == 0 && dg >= -8 && dg <= 7 && db >= -8 && db <= 7) {
							encoded[encodedOffset++] = (byte) (208 + dr);
							encoded[encodedOffset++] = (byte) (136 + (dg << 4) + db);
						}
						else {
							dr += 16;
							encoded[encodedOffset++] = (byte) (224 + (dr >> 1));
							db += 16;
							encoded[encodedOffset++] = (byte) (((dr & 1) << 7) + ((dg + 16) << 2) + (db >> 3));
							encoded[encodedOffset++] = (byte) (((db & 7) << 5) + da + 16);
						}
					}
					else {
						encoded[encodedOffset++] = (byte) (240 | (dr != 0 ? 8 : 0) | (dg != 0 ? 4 : 0) | (db != 0 ? 2 : 0) | (da != 0 ? 1 : 0));
						if (dr != 0)
							encoded[encodedOffset++] = (byte) r;
						if (dg != 0)
							encoded[encodedOffset++] = (byte) g;
						if (db != 0)
							encoded[encodedOffset++] = (byte) b;
						if (da != 0)
							encoded[encodedOffset++] = (byte) a;
					}
				}
				lastPixel = pixel;
			}
		}
		Array.Clear(encoded, encodedOffset, 4);
		this.Encoded = encoded;
		this.EncodedSize = encodedOffset + 4;
		return true;
	}

	public byte[] GetEncoded()
	{
		return this.Encoded;
	}

	public int GetEncodedSize()
	{
		return this.EncodedSize;
	}
}

public class QOIDecoder
{
	public QOIDecoder()
	{
	}

	int Width;

	int Height;

	int[] Pixels;

	bool Alpha;

	int Colorspace;

	public bool Decode(byte[] encoded, int encodedSize)
	{
		if (encoded == null || encodedSize < 19 || encoded[0] != 113 || encoded[1] != 111 || encoded[2] != 105 || encoded[3] != 102)
			return false;
		int width = encoded[4] << 24 | encoded[5] << 16 | encoded[6] << 8 | encoded[7];
		int height = encoded[8] << 24 | encoded[9] << 16 | encoded[10] << 8 | encoded[11];
		if (width <= 0 || height <= 0 || height > 2147483647 / width)
			return false;
		int pixelsSize = width * height;
		int[] pixels = new int[pixelsSize];
		encodedSize -= 4;
		int encodedOffset = 14;
		int[] index = new int[64];
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
					Array.Fill(pixels, pixel, pixelsOffset, run);
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
		this.Width = width;
		this.Height = height;
		this.Pixels = pixels;
		this.Alpha = encoded[12] == 4;
		this.Colorspace = encoded[13];
		return true;
	}

	public int GetWidth()
	{
		return this.Width;
	}

	public int GetHeight()
	{
		return this.Height;
	}

	public int[] GetPixels()
	{
		return this.Pixels;
	}

	public bool GetAlpha()
	{
		return this.Alpha;
	}

	public int GetColorspace()
	{
		return this.Colorspace;
	}
}
