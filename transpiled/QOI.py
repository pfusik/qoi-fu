# Generated automatically with "fut". Do not edit.
import array

class QOIEncoder:
	"""Encoder of the "Quite OK Image" (QOI) format.

	Losslessly compresses an image to a byte array."""

	def __init__(self):
		"""Constructs the encoder.

		The encoder can be used for several images, one after another."""

	_HEADER_SIZE = 14

	_PADDING_SIZE = 8
	_encoded: bytearray | None
	_encoded_size: int

	@staticmethod
	def can_encode(width: int, height: int, alpha: bool) -> bool:
		"""Determines if an image of given size can be encoded.

		:param width: Image width in pixels.
		:param height: Image height in pixels.
		:param alpha: Whether the image has the alpha channel (transparency).
		"""
		return width > 0 and height > 0 and height <= int(int(2147483625 / width) / (5 if alpha else 4))

	def encode(self, width: int, height: int, pixels: array.array, alpha: bool, linear_colorspace: bool) -> bool:
		"""Encodes the given image.

		Returns `True` if encoded successfully.

		:param width: Image width in pixels.
		:param height: Image height in pixels.
		:param pixels: Pixels of the image, top-down, left-to-right.
		:param alpha: `False` specifies that all pixels are opaque. High bytes of `pixels` elements are ignored then.
		:param linear_colorspace: Specifies the color space.
		"""
		if not QOIEncoder.can_encode(width, height, alpha):
			return False
		pixels_size: int = width * height
		encoded: bytearray = bytearray(14 + pixels_size * (5 if alpha else 4) + 8)
		encoded[0] = 113
		encoded[1] = 111
		encoded[2] = 105
		encoded[3] = 102
		encoded[4] = width >> 24
		encoded[5] = width >> 16 & 255
		encoded[6] = width >> 8 & 255
		encoded[7] = width & 255
		encoded[8] = height >> 24
		encoded[9] = height >> 16 & 255
		encoded[10] = height >> 8 & 255
		encoded[11] = height & 255
		encoded[12] = 4 if alpha else 3
		encoded[13] = 1 if linear_colorspace else 0
		index: array.array = array.array("i", [ 0 ]) * 64
		encoded_offset: int = 14
		last_pixel: int = -16777216
		run: int = 0
		pixels_offset: int = 0
		while pixels_offset < pixels_size:
			pixel: int = pixels[pixels_offset]
			pixels_offset += 1
			if not alpha:
				pixel |= -16777216
			if pixel == last_pixel:
				run += 1
				if run == 62 or pixels_offset >= pixels_size:
					encoded[encoded_offset] = 191 + run
					encoded_offset += 1
					run = 0
			else:
				if run > 0:
					encoded[encoded_offset] = 191 + run
					encoded_offset += 1
					run = 0
				index_offset: int = ((pixel >> 16) * 3 + (pixel >> 8) * 5 + (pixel & 63) * 7 + (pixel >> 24) * 11) & 63
				if pixel == index[index_offset]:
					encoded[encoded_offset] = index_offset
					encoded_offset += 1
				else:
					index[index_offset] = pixel
					r: int = pixel >> 16 & 255
					g: int = pixel >> 8 & 255
					b: int = pixel & 255
					if (pixel ^ last_pixel) >> 24 != 0:
						encoded[encoded_offset] = 255
						encoded[encoded_offset + 1] = r
						encoded[encoded_offset + 2] = g
						encoded[encoded_offset + 3] = b
						encoded[encoded_offset + 4] = pixel >> 24 & 255
						encoded_offset += 5
					else:
						dr: int = (((r - (last_pixel >> 16)) & 255) ^ 128) - 128
						dg: int = (((g - (last_pixel >> 8)) & 255) ^ 128) - 128
						db: int = (((b - last_pixel) & 255) ^ 128) - 128
						if dr >= -2 and dr <= 1 and dg >= -2 and dg <= 1 and db >= -2 and db <= 1:
							encoded[encoded_offset] = 106 + (dr << 4) + (dg << 2) + db
							encoded_offset += 1
						else:
							dr -= dg
							db -= dg
							if dr >= -8 and dr <= 7 and dg >= -32 and dg <= 31 and db >= -8 and db <= 7:
								encoded[encoded_offset] = 160 + dg
								encoded[encoded_offset + 1] = 136 + (dr << 4) + db
								encoded_offset += 2
							else:
								encoded[encoded_offset] = 254
								encoded[encoded_offset + 1] = r
								encoded[encoded_offset + 2] = g
								encoded[encoded_offset + 3] = b
								encoded_offset += 4
				last_pixel = pixel
		encoded[encoded_offset:encoded_offset + 7] = bytearray(7)
		encoded[encoded_offset + 8 - 1] = 1
		self._encoded = encoded
		self._encoded_size = encoded_offset + 8
		return True

	def get_encoded(self) -> bytearray | bytes:
		"""Returns the encoded file contents.

		This method can only be called after `Encode` returned `True`.
		The allocated array is usually larger than the encoded data.
		Call `GetEncodedSize` to retrieve the number of leading bytes that are significant."""
		return self._encoded

	def get_encoded_size(self) -> int:
		"""Returns the encoded file length."""
		return self._encoded_size

class QOIDecoder:
	"""Decoder of the "Quite OK Image" (QOI) format."""

	def __init__(self):
		"""Constructs the decoder.

		The decoder can be used for several images, one after another."""
	_width: int
	_height: int
	_pixels: array.array | None
	_alpha: bool
	_linear_colorspace: bool

	def decode(self, encoded: bytearray | bytes, encoded_size: int) -> bool:
		"""Decodes the given QOI file contents.

		Returns `True` if decoded successfully.

		:param encoded: QOI file contents. Only the first `encodedSize` bytes are accessed.
		:param encoded_size: QOI file length.
		"""
		if encoded_size < 23 or encoded[0] != 113 or encoded[1] != 111 or encoded[2] != 105 or encoded[3] != 102:
			return False
		width: int = encoded[4] << 24 | encoded[5] << 16 | encoded[6] << 8 | encoded[7]
		height: int = encoded[8] << 24 | encoded[9] << 16 | encoded[10] << 8 | encoded[11]
		if width <= 0 or height <= 0 or height > int(2147483647 / width):
			return False
		match encoded[12]:
			case 3:
				self._alpha = False
			case 4:
				self._alpha = True
			case _:
				return False
		match encoded[13]:
			case 0:
				self._linear_colorspace = False
			case 1:
				self._linear_colorspace = True
			case _:
				return False
		pixels_size: int = width * height
		pixels: array.array = array.array("i", [ 0 ]) * pixels_size
		encoded_size -= 8
		encoded_offset: int = 14
		index: array.array = array.array("i", [ 0 ]) * 64
		pixel: int = -16777216
		pixels_offset: int = 0
		while pixels_offset < pixels_size:
			if encoded_offset >= encoded_size:
				return False
			e: int = encoded[encoded_offset]
			encoded_offset += 1
			match e >> 6:
				case 0:
					pixels[pixels_offset] = pixel = index[e]
					pixels_offset += 1
					continue
				case 1:
					pixel = (pixel & -16777216) | ((pixel + (((e >> 4) - 4 - 2) << 16)) & 16711680) | ((pixel + (((e >> 2 & 3) - 2) << 8)) & 65280) | ((pixel + (e & 3) - 2) & 255)
				case 2:
					e -= 160
					rb: int = encoded[encoded_offset]
					encoded_offset += 1
					pixel = (pixel & -16777216) | ((pixel + ((e + (rb >> 4) - 8) << 16)) & 16711680) | ((pixel + (e << 8)) & 65280) | ((pixel + e + (rb & 15) - 8) & 255)
				case _:
					if e < 254:
						e -= 191
						if pixels_offset + e > pixels_size:
							return False
						if pixels_offset == 0:
							index[53] = pixel
						pixels[pixels_offset:pixels_offset + e] = array.array("i", [ pixel ]) * e
						pixels_offset += e
						continue
					if e == 254:
						pixel = (pixel & -16777216) | encoded[encoded_offset] << 16 | encoded[encoded_offset + 1] << 8 | encoded[encoded_offset + 2]
						encoded_offset += 3
					else:
						pixel = encoded[encoded_offset + 3] << 24 | encoded[encoded_offset] << 16 | encoded[encoded_offset + 1] << 8 | encoded[encoded_offset + 2]
						encoded_offset += 4
			pixels[pixels_offset] = index[((pixel >> 16) * 3 + (pixel >> 8) * 5 + (pixel & 63) * 7 + (pixel >> 24) * 11) & 63] = pixel
			pixels_offset += 1
		if encoded_offset != encoded_size:
			return False
		self._width = width
		self._height = height
		self._pixels = pixels
		return True

	def get_width(self) -> int:
		"""Returns the width of the decoded image in pixels."""
		return self._width

	def get_height(self) -> int:
		"""Returns the height of the decoded image in pixels."""
		return self._height

	def get_pixels(self) -> array.array:
		"""Returns the pixels of the decoded image, top-down, left-to-right.

		Each pixel is a 32-bit integer 0xAARRGGBB."""
		return self._pixels

	def has_alpha(self) -> bool:
		"""Returns the information about the alpha channel from the file header."""
		return self._alpha

	def is_linear_colorspace(self) -> bool:
		"""Returns the color space information from the file header.

		`False` = sRGB with linear alpha channel.
		`True` = all channels linear."""
		return self._linear_colorspace
