// Generated automatically with "fut". Do not edit.
import java.util.Arrays;

/**
 * Decoder of the "Quite OK Image" (QOI) format.
 */
public class QOIDecoder
{
	/**
	 * Constructs the decoder.
	 * The decoder can be used for several images, one after another.
	 */
	public QOIDecoder()
	{
	}
	private int width;
	private int height;
	private int[] pixels;
	private boolean alpha;
	private boolean linearColorspace;

	/**
	 * Decodes the given QOI file contents.
	 * Returns <code>true</code> if decoded successfully.
	 * @param encoded QOI file contents. Only the first <code>encodedSize</code> bytes are accessed.
	 * @param encodedSize QOI file length.
	 */
	public final boolean decode(byte[] encoded, int encodedSize)
	{
		if (encodedSize < 23 || encoded[0] != 'q' || encoded[1] != 'o' || encoded[2] != 'i' || encoded[3] != 'f')
			return false;
		int width = (encoded[4] & 0xff) << 24 | (encoded[5] & 0xff) << 16 | (encoded[6] & 0xff) << 8 | encoded[7] & 0xff;
		int height = (encoded[8] & 0xff) << 24 | (encoded[9] & 0xff) << 16 | (encoded[10] & 0xff) << 8 | encoded[11] & 0xff;
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
		final int[] index = new int[64];
		int pixel = -16777216;
		for (int pixelsOffset = 0; pixelsOffset < pixelsSize;) {
			if (encodedOffset >= encodedSize)
				return false;
			int e = encoded[encodedOffset++] & 0xff;
			switch (e >> 6) {
			case 0:
				pixels[pixelsOffset++] = pixel = index[e];
				continue;
			case 1:
				pixel = (pixel & -16777216) | ((pixel + (((e >> 4) - 4 - 2) << 16)) & 16711680) | ((pixel + (((e >> 2 & 3) - 2) << 8)) & 65280) | ((pixel + (e & 3) - 2) & 255);
				break;
			case 2:
				e -= 160;
				int rb = encoded[encodedOffset++] & 0xff;
				pixel = (pixel & -16777216) | ((pixel + ((e + (rb >> 4) - 8) << 16)) & 16711680) | ((pixel + (e << 8)) & 65280) | ((pixel + e + (rb & 15) - 8) & 255);
				break;
			default:
				if (e < 254) {
					e -= 191;
					if (pixelsOffset + e > pixelsSize)
						return false;
					Arrays.fill(pixels, pixelsOffset, pixelsOffset + e, pixel);
					pixelsOffset += e;
					continue;
				}
				if (e == 254) {
					pixel = (pixel & -16777216) | (encoded[encodedOffset] & 0xff) << 16 | (encoded[encodedOffset + 1] & 0xff) << 8 | encoded[encodedOffset + 2] & 0xff;
					encodedOffset += 3;
				}
				else {
					pixel = (encoded[encodedOffset + 3] & 0xff) << 24 | (encoded[encodedOffset] & 0xff) << 16 | (encoded[encodedOffset + 1] & 0xff) << 8 | encoded[encodedOffset + 2] & 0xff;
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

	/**
	 * Returns the width of the decoded image in pixels.
	 */
	public final int getWidth()
	{
		return this.width;
	}

	/**
	 * Returns the height of the decoded image in pixels.
	 */
	public final int getHeight()
	{
		return this.height;
	}

	/**
	 * Returns the pixels of the decoded image, top-down, left-to-right.
	 * Each pixel is a 32-bit integer 0xAARRGGBB.
	 */
	public final int[] getPixels()
	{
		return this.pixels;
	}

	/**
	 * Returns the information about the alpha channel from the file header.
	 */
	public final boolean hasAlpha()
	{
		return this.alpha;
	}

	/**
	 * Returns the color space information from the file header.
	 * <code>false</code> = sRGB with linear alpha channel.
	 * <code>true</code> = all channels linear.
	 */
	public final boolean isLinearColorspace()
	{
		return this.linearColorspace;
	}
}
