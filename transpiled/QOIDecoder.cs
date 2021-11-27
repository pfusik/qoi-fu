// Generated automatically with "cito". Do not edit.
using System;

public class QOIDecoder
{
	public QOIDecoder()
	{
	}

	int Width;

	int Height;

	int[] Pixels;

	public bool Decode(byte[] encoded, int encodedSize)
	{
		if (encoded == null || encodedSize < 17 || encoded[0] != 113 || encoded[1] != 111 || encoded[2] != 105 || encoded[3] != 102)
			return false;
		int dataSize = encoded[8] << 24 | encoded[9] << 16 | encoded[10] << 8 | encoded[11];
		if (12 + dataSize != encodedSize)
			return false;
		int width = encoded[4] << 8 | encoded[5];
		int height = encoded[6] << 8 | encoded[7];
		if (height > 2147483647 / width)
			return false;
		int pixelsSize = width * height;
		int[] pixels = new int[pixelsSize];
		encodedSize -= 4;
		int encodedOffset = 12;
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
					pixel = (pixel & -16777216) | ((pixel + (((e >> 4) - 8 - 1) << 16)) & 16711680) | ((pixel + (((e >> 2 & 3) - 1) << 8)) & 65280) | ((pixel + (e & 3) - 1) & 255);
				else {
					int d = encoded[encodedOffset++];
					pixel = (pixel & -16777216) | ((pixel + ((e - 192 - 15) << 16)) & 16711680) | ((pixel + (((d >> 4) - 7) << 8)) & 65280) | ((pixel + (d & 15) - 7) & 255);
				}
			}
			else if (e < 240) {
				e = e << 16 | encoded[encodedOffset] << 8 | encoded[encodedOffset + 1];
				encodedOffset += 2;
				pixel = ((pixel + (((e & 31) - 15) << 24)) & -16777216) | ((pixel + (((e >> 15) - 448 - 15) << 16)) & 16711680) | ((pixel + (((e >> 10 & 31) - 15) << 8)) & 65280) | ((pixel + (e >> 5 & 31) - 15) & 255);
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
}
