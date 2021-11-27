// Generated automatically with "cito". Do not edit.

"use strict";

function QOIDecoder()
{
}

QOIDecoder.prototype.decode = function(encoded, encodedSize)
{
	if (encoded == null || encodedSize < 17 || encoded[0] != 113 || encoded[1] != 111 || encoded[2] != 105 || encoded[3] != 102)
		return false;
	let dataSize = encoded[8] << 24 | encoded[9] << 16 | encoded[10] << 8 | encoded[11];
	if (12 + dataSize != encodedSize)
		return false;
	let width = encoded[4] << 8 | encoded[5];
	let height = encoded[6] << 8 | encoded[7];
	if (height > (2147483647 / width | 0))
		return false;
	let pixelsSize = width * height;
	let pixels = new Int32Array(pixelsSize);
	encodedSize -= 4;
	let encodedOffset = 12;
	const index = new Int32Array(64);
	let pixel = -16777216;
	for (let pixelsOffset = 0; pixelsOffset < pixelsSize;) {
		if (encodedOffset >= encodedSize)
			return false;
		let e = encoded[encodedOffset++];
		if (e < 128) {
			if (e < 64)
				pixels[pixelsOffset++] = pixel = index[e];
			else {
				let run = 1 + (e & 31);
				if (e >= 96)
					run += 32 + encoded[encodedOffset++];
				if (pixelsOffset + run > pixelsSize)
					return false;
				pixels.fill(pixel, pixelsOffset, pixelsOffset + run);
				pixelsOffset += run;
			}
			continue;
		}
		else if (e < 224) {
			if (e < 192)
				pixel = (pixel & -16777216) | ((pixel + (((e >> 4) - 8 - 1) << 16)) & 16711680) | ((pixel + (((e >> 2 & 3) - 1) << 8)) & 65280) | ((pixel + (e & 3) - 1) & 255);
			else {
				let d = encoded[encodedOffset++];
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
	this.width = width;
	this.height = height;
	this.pixels = pixels;
	return true;
}

QOIDecoder.prototype.getWidth = function()
{
	return this.width;
}

QOIDecoder.prototype.getHeight = function()
{
	return this.height;
}

QOIDecoder.prototype.getPixels = function()
{
	return this.pixels;
}
