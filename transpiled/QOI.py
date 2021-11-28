# Generated automatically with "cito". Do not edit.
import array

class QOIColorspace:

	SRGB = 0

	SRGB_LINEAR_ALPHA = 1

	LINEAR = 15

class QOIEncoder:

	_HEADER_SIZE = 14

	_PADDING_SIZE = 4

	def __init__(self):
		pass

	def encode(self, width, height, pixels, alpha, colorspace):
		if width <= 0 or height <= 0 or height > int(429496725 / width) or pixels is None:
			return False
		pixels_size = width * height
		encoded = bytearray(14 + pixels_size * (5 if alpha else 4) + 4)
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
		encoded[13] = colorspace
		index = array.array("i", [ 0 ]) * 64
		encoded_offset = 14
		last_pixel = -16777216
		run = 0
		pixels_offset = 0
		while pixels_offset < pixels_size:
			pixel = pixels[pixels_offset]
			pixels_offset += 1
			if not alpha:
				pixel |= -16777216
			if pixel == last_pixel:
				run += 1
			if run > 0 and (pixel != last_pixel or run == 8224 or pixels_offset >= pixels_size):
				if run <= 32:
					encoded[encoded_offset] = 64 + run - 1
					encoded_offset += 1
				else:
					run -= 33
					encoded[encoded_offset] = 96 + (run >> 8)
					encoded_offset += 1
					encoded[encoded_offset] = run & 255
					encoded_offset += 1
				run = 0
			if pixel != last_pixel:
				index_offset = (pixel >> 24 ^ pixel >> 16 ^ pixel >> 8 ^ pixel) & 63
				if pixel == index[index_offset]:
					encoded[encoded_offset] = index_offset
					encoded_offset += 1
				else:
					index[index_offset] = pixel
					r = pixel >> 16 & 255
					g = pixel >> 8 & 255
					b = pixel & 255
					a = pixel >> 24 & 255
					dr = r - (last_pixel >> 16 & 255)
					dg = g - (last_pixel >> 8 & 255)
					db = b - (last_pixel & 255)
					da = a - (last_pixel >> 24 & 255)
					if dr >= -16 and dr <= 15 and dg >= -16 and dg <= 15 and db >= -16 and db <= 15 and da >= -16 and da <= 15:
						if da == 0 and dr >= -2 and dr <= 1 and dg >= -2 and dg <= 1 and db >= -2 and db <= 1:
							encoded[encoded_offset] = 170 + (dr << 4) + (dg << 2) + db
							encoded_offset += 1
						elif da == 0 and dg >= -8 and dg <= 7 and db >= -8 and db <= 7:
							encoded[encoded_offset] = 208 + dr
							encoded_offset += 1
							encoded[encoded_offset] = 136 + (dg << 4) + db
							encoded_offset += 1
						else:
							dr += 16
							encoded[encoded_offset] = 224 + (dr >> 1)
							encoded_offset += 1
							db += 16
							encoded[encoded_offset] = ((dr & 1) << 7) + ((dg + 16) << 2) + (db >> 3)
							encoded_offset += 1
							encoded[encoded_offset] = ((db & 7) << 5) + da + 16
							encoded_offset += 1
					else:
						encoded[encoded_offset] = 240 | (8 if dr != 0 else 0) | (4 if dg != 0 else 0) | (2 if db != 0 else 0) | (1 if da != 0 else 0)
						encoded_offset += 1
						if dr != 0:
							encoded[encoded_offset] = r
							encoded_offset += 1
						if dg != 0:
							encoded[encoded_offset] = g
							encoded_offset += 1
						if db != 0:
							encoded[encoded_offset] = b
							encoded_offset += 1
						if da != 0:
							encoded[encoded_offset] = a
							encoded_offset += 1
				last_pixel = pixel
		encoded[encoded_offset:encoded_offset + 4] = bytearray(4)
		self._encoded = encoded
		self._encoded_size = encoded_offset + 4
		return True

	def get_encoded(self):
		return self._encoded

	def get_encoded_size(self):
		return self._encoded_size

class QOIDecoder:

	def __init__(self):
		pass

	def decode(self, encoded, encoded_size):
		if encoded is None or encoded_size < 19 or encoded[0] != 113 or encoded[1] != 111 or encoded[2] != 105 or encoded[3] != 102:
			return False
		width = encoded[4] << 24 | encoded[5] << 16 | encoded[6] << 8 | encoded[7]
		height = encoded[8] << 24 | encoded[9] << 16 | encoded[10] << 8 | encoded[11]
		if width <= 0 or height <= 0 or height > int(2147483647 / width):
			return False
		pixels_size = width * height
		pixels = array.array("i", [ 0 ]) * pixels_size
		encoded_size -= 4
		encoded_offset = 14
		index = array.array("i", [ 0 ]) * 64
		pixel = -16777216
		pixels_offset = 0
		while pixels_offset < pixels_size:
			if encoded_offset >= encoded_size:
				return False
			e = encoded[encoded_offset]
			encoded_offset += 1
			if e < 128:
				if e < 64:
					pixels[pixels_offset] = pixel = index[e]
					pixels_offset += 1
				else:
					run = 1 + (e & 31)
					if e >= 96:
						run += 32 + encoded[encoded_offset]
						encoded_offset += 1
					if pixels_offset + run > pixels_size:
						return False
					pixels[pixels_offset:pixels_offset + run] = array.array("i", [ pixel ]) * run
					pixels_offset += run
				continue
			elif e < 224:
				if e < 192:
					pixel = (pixel & -16777216) | ((pixel + (((e >> 4) - 8 - 2) << 16)) & 16711680) | ((pixel + (((e >> 2 & 3) - 2) << 8)) & 65280) | ((pixel + (e & 3) - 2) & 255)
				else:
					d = encoded[encoded_offset]
					encoded_offset += 1
					pixel = (pixel & -16777216) | ((pixel + ((e - 192 - 16) << 16)) & 16711680) | ((pixel + (((d >> 4) - 8) << 8)) & 65280) | ((pixel + (d & 15) - 8) & 255)
			elif e < 240:
				e = e << 16 | encoded[encoded_offset] << 8 | encoded[encoded_offset + 1]
				encoded_offset += 2
				pixel = ((pixel + (((e & 31) - 16) << 24)) & -16777216) | ((pixel + (((e >> 15) - 448 - 16) << 16)) & 16711680) | ((pixel + (((e >> 10 & 31) - 16) << 8)) & 65280) | ((pixel + (e >> 5 & 31) - 16) & 255)
			else:
				if (e & 8) != 0:
					pixel = (pixel & -16711681) | encoded[encoded_offset] << 16
					encoded_offset += 1
				if (e & 4) != 0:
					pixel = (pixel & -65281) | encoded[encoded_offset] << 8
					encoded_offset += 1
				if (e & 2) != 0:
					pixel = (pixel & -256) | encoded[encoded_offset]
					encoded_offset += 1
				if (e & 1) != 0:
					pixel = (pixel & 16777215) | encoded[encoded_offset] << 24
					encoded_offset += 1
			pixels[pixels_offset] = index[(pixel >> 24 ^ pixel >> 16 ^ pixel >> 8 ^ pixel) & 63] = pixel
			pixels_offset += 1
		if encoded_offset != encoded_size:
			return False
		self._width = width
		self._height = height
		self._pixels = pixels
		self._alpha = encoded[12] == 4
		self._colorspace = encoded[13]
		return True

	def get_width(self):
		return self._width

	def get_height(self):
		return self._height

	def get_pixels(self):
		return self._pixels

	def get_alpha(self):
		return self._alpha

	def get_colorspace(self):
		return self._colorspace
