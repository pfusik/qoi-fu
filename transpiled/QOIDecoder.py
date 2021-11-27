# Generated automatically with "cito". Do not edit.
import array

class QOIDecoder:

	def __init__(self):
		pass

	def decode(self, encoded, encoded_size):
		if encoded is None or encoded_size < 17 or encoded[0] != 113 or encoded[1] != 111 or encoded[2] != 105 or encoded[3] != 102:
			return False
		data_size = encoded[8] << 24 | encoded[9] << 16 | encoded[10] << 8 | encoded[11]
		if 12 + data_size != encoded_size:
			return False
		width = encoded[4] << 8 | encoded[5]
		height = encoded[6] << 8 | encoded[7]
		if height > int(2147483647 / width):
			return False
		pixels_size = width * height
		pixels = array.array("i", [ 0 ]) * pixels_size
		encoded_size -= 4
		encoded_offset = 12
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
					pixel = (pixel & -16777216) | ((pixel + (((e >> 4) - 8 - 1) << 16)) & 16711680) | ((pixel + (((e >> 2 & 3) - 1) << 8)) & 65280) | ((pixel + (e & 3) - 1) & 255)
				else:
					d = encoded[encoded_offset]
					encoded_offset += 1
					pixel = (pixel & -16777216) | ((pixel + ((e - 192 - 15) << 16)) & 16711680) | ((pixel + (((d >> 4) - 7) << 8)) & 65280) | ((pixel + (d & 15) - 7) & 255)
			elif e < 240:
				e = e << 16 | encoded[encoded_offset] << 8 | encoded[encoded_offset + 1]
				encoded_offset += 2
				pixel = ((pixel + (((e & 31) - 15) << 24)) & -16777216) | ((pixel + (((e >> 15) - 448 - 15) << 16)) & 16711680) | ((pixel + (((e >> 10 & 31) - 15) << 8)) & 65280) | ((pixel + (e >> 5 & 31) - 15) & 255)
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
		return True

	def get_width(self):
		return self._width

	def get_height(self):
		return self._height

	def get_pixels(self):
		return self._pixels
