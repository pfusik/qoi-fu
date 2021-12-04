// Generated automatically with "cito". Do not edit.
import java.util.Arrays;

public class QOIEncoder
{
	public QOIEncoder()
	{
	}

	static final int HEADER_SIZE = 14;

	static final int PADDING_SIZE = 4;
	private byte[] encoded;
	private int encodedSize;

	public static boolean canEncode(int width, int height, boolean alpha)
	{
		return width > 0 && height > 0 && height <= 2147483629 / width / (alpha ? 5 : 4);
	}

	public final boolean encode(int width, int height, int[] pixels, boolean alpha, int colorspace)
	{
		if (pixels == null || !canEncode(width, height, alpha))
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
		final int[] index = new int[64];
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
		Arrays.fill(encoded, encodedOffset, encodedOffset + 4, (byte) 0);
		this.encoded = encoded;
		this.encodedSize = encodedOffset + 4;
		return true;
	}

	public final byte[] getEncoded()
	{
		return this.encoded;
	}

	public final int getEncodedSize()
	{
		return this.encodedSize;
	}
}
