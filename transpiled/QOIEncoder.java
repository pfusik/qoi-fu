// Generated automatically with "cito". Do not edit.
import java.util.Arrays;

public class QOIEncoder
{
	public QOIEncoder()
	{
	}

	private static final int HEADER_SIZE = 12;
	private byte[] encoded;
	private int encodedSize;

	public final boolean encode(int width, int height, int[] pixels, boolean alpha)
	{
		if (width <= 0 || width >= 65536 || height <= 0 || height >= 65536 || height > 429496726 / width || pixels == null)
			return false;
		int pixelsSize = width * height;
		byte[] encoded = new byte[12 + pixelsSize * (alpha ? 5 : 4) + 4];
		encoded[0] = 113;
		encoded[1] = 111;
		encoded[2] = 105;
		encoded[3] = 102;
		encoded[4] = (byte) (width >> 8);
		encoded[5] = (byte) width;
		encoded[6] = (byte) (height >> 8);
		encoded[7] = (byte) height;
		final int[] index = new int[64];
		int encodedOffset = 12;
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
					if (dr >= -15 && dr <= 16 && dg >= -15 && dg <= 16 && db >= -15 && db <= 16 && da >= -15 && da <= 16) {
						if (da == 0 && dr >= -1 && dr <= 2 && dg >= -1 && dg <= 2 && db >= -1 && db <= 2)
							encoded[encodedOffset++] = (byte) (149 + (dr << 4) + (dg << 2) + db);
						else if (da == 0 && dg >= -7 && dg <= 8 && db >= -7 && db <= 8) {
							encoded[encodedOffset++] = (byte) (207 + dr);
							encoded[encodedOffset++] = (byte) (119 + (dg << 4) + db);
						}
						else {
							dr += 15;
							encoded[encodedOffset++] = (byte) (224 + (dr >> 1));
							db += 15;
							encoded[encodedOffset++] = (byte) (((dr & 1) << 7) + ((dg + 15) << 2) + (db >> 3));
							encoded[encodedOffset++] = (byte) (((db & 7) << 5) + da + 15);
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
		this.encodedSize = encodedOffset + 4;
		int dataSize = this.encodedSize - 12;
		encoded[8] = (byte) (dataSize >> 24);
		encoded[9] = (byte) (dataSize >> 16);
		encoded[10] = (byte) (dataSize >> 8);
		encoded[11] = (byte) dataSize;
		this.encoded = encoded;
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
