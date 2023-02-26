// Generated automatically with "cito". Do not edit.
import java.util.Arrays;

/**
 * Encoder of the "Quite OK Image" (QOI) format.
 * Losslessly compresses an image to a byte array.
 */
public class QOIEncoder
{
	/**
	 * Constructs the encoder.
	 * The encoder can be used for several images, one after another.
	 */
	public QOIEncoder()
	{
	}

	static final int HEADER_SIZE = 14;

	static final int PADDING_SIZE = 8;
	private byte[] encoded;
	private int encodedSize;

	/**
	 * Determines if an image of given size can be encoded.
	 * @param width Image width in pixels.
	 * @param height Image height in pixels.
	 * @param alpha Whether the image has the alpha channel (transparency).
	 */
	public static boolean canEncode(int width, int height, boolean alpha)
	{
		return width > 0 && height > 0 && height <= 2147483625 / width / (alpha ? 5 : 4);
	}

	/**
	 * Encodes the given image.
	 * Returns <code>true</code> if encoded successfully.
	 * @param width Image width in pixels.
	 * @param height Image height in pixels.
	 * @param pixels Pixels of the image, top-down, left-to-right.
	 * @param alpha <code>false</code> specifies that all pixels are opaque. High bytes of <code>pixels</code> elements are ignored then.
	 * @param linearColorspace Specifies the color space.
	 */
	public final boolean encode(int width, int height, int[] pixels, boolean alpha, boolean linearColorspace)
	{
		if (!canEncode(width, height, alpha))
			return false;
		int pixelsSize = width * height;
		byte[] encoded = new byte[14 + pixelsSize * (alpha ? 5 : 4) + 8];
		encoded[0] = 'q';
		encoded[1] = 'o';
		encoded[2] = 'i';
		encoded[3] = 'f';
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
		final int[] index = new int[64];
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
					int a = pixel >> 24 & 255;
					if ((pixel ^ lastPixel) >> 24 != 0) {
						encoded[encodedOffset] = (byte) 255;
						encoded[encodedOffset + 1] = (byte) r;
						encoded[encodedOffset + 2] = (byte) g;
						encoded[encodedOffset + 3] = (byte) b;
						encoded[encodedOffset + 4] = (byte) a;
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
								encoded[encodedOffset] = (byte) 254;
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
		Arrays.fill(encoded, encodedOffset, encodedOffset + 7, (byte) 0);
		encoded[encodedOffset + 8 - 1] = 1;
		this.encoded = encoded;
		this.encodedSize = encodedOffset + 8;
		return true;
	}

	/**
	 * Returns the encoded file contents.
	 * This method can only be called after <code>Encode</code> returned <code>true</code>.
	 * The allocated array is usually larger than the encoded data.
	 * Call <code>GetEncodedSize</code> to retrieve the number of leading bytes that are significant.
	 */
	public final byte[] getEncoded()
	{
		return this.encoded;
	}

	/**
	 * Returns the encoded file length.
	 */
	public final int getEncodedSize()
	{
		return this.encodedSize;
	}
}
