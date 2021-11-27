# Generated automatically with "cito". Do not edit.
import array

class QOIEncoder:

	def __init__(self):
		pass

	def encode(self, width, height, pixels, alpha):
		if width <= 0 or width >= 65536 or height <= 0 or height >= 65536 or height > int(429496726 / width) or pixels is None:
			return False
		pixels_size = width * height
		encoded = bytearray(12 + pixels_size * (5 if alpha else 4) + 4)
		encoded[0] = 113
		encoded[1] = 111
		encoded[2] = 105
		encoded[3] = 102
		encoded[4] = width >> 8
		encoded[5] = width & 255
		encoded[6] = height >> 8
		encoded[7] = height & 255
		index = array.array("i", [ 0 ]) * 64
		encoded_offset = 12
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
					if dr >= -15 and dr <= 16 and dg >= -15 and dg <= 16 and db >= -15 and db <= 16 and da >= -15 and da <= 16:
						if da == 0 and dr >= -1 and dr <= 2 and dg >= -1 and dg <= 2 and db >= -1 and db <= 2:
							encoded[encoded_offset] = 149 + (dr << 4) + (dg << 2) + db
							encoded_offset += 1
						elif da == 0 and dg >= -7 and dg <= 8 and db >= -7 and db <= 8:
							encoded[encoded_offset] = 207 + dr
							encoded_offset += 1
							encoded[encoded_offset] = 119 + (dg << 4) + db
							encoded_offset += 1
						else:
							dr += 15
							encoded[encoded_offset] = 224 + (dr >> 1)
							encoded_offset += 1
							db += 15
							encoded[encoded_offset] = ((dr & 1) << 7) + ((dg + 15) << 2) + (db >> 3)
							encoded_offset += 1
							encoded[encoded_offset] = ((db & 7) << 5) + da + 15
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
		self._encoded_size = encoded_offset + 4
		data_size = self._encoded_size - 12
		encoded[8] = data_size >> 24
		encoded[9] = data_size >> 16 & 255
		encoded[10] = data_size >> 8 & 255
		encoded[11] = data_size & 255
		self._encoded = encoded
		return True

	def get_encoded(self):
		return self._encoded

	def get_encoded_size(self):
		return self._encoded_size
