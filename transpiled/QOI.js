// Generated automatically with "cito". Do not edit.

"use strict";

function QOIColorspace()
{
}

QOIColorspace.SRGB = 0;

QOIColorspace.SRGB_LINEAR_ALPHA = 1;

QOIColorspace.LINEAR = 15;

function QOIEncoder()
{
}

QOIEncoder.HEADER_SIZE = 14;

QOIEncoder.PADDING_SIZE = 4;

QOIEncoder.canEncode = function(width, height, alpha)
{
	return width > 0 && height > 0 && height <= ((2147483629 / width | 0) / (alpha ? 5 : 4) | 0);
}

QOIEncoder.prototype.encode = function(width, height, pixels, alpha, colorspace)
{
	if (pixels == null || !QOIEncoder.canEncode(width, height, alpha))
		return false;
	let pixelsSize = width * height;
	let encoded = new Uint8Array(14 + pixelsSize * (alpha ? 5 : 4) + 4);
	encoded[0] = 113;
	encoded[1] = 111;
	encoded[2] = 105;
	encoded[3] = 102;
	encoded[4] = width >> 24;
	encoded[5] = width >> 16 & 255;
	encoded[6] = width >> 8 & 255;
	encoded[7] = width & 255;
	encoded[8] = height >> 24;
	encoded[9] = height >> 16 & 255;
	encoded[10] = height >> 8 & 255;
	encoded[11] = height & 255;
	encoded[12] = alpha ? 4 : 3;
	encoded[13] = colorspace;
	const index = new Int32Array(64);
	let encodedOffset = 14;
	let lastPixel = -16777216;
	let run = 0;
	for (let pixelsOffset = 0; pixelsOffset < pixelsSize;) {
		let pixel = pixels[pixelsOffset++];
		if (!alpha)
			pixel |= -16777216;
		if (pixel == lastPixel)
			run++;
		if (run > 0 && (pixel != lastPixel || run == 8224 || pixelsOffset >= pixelsSize)) {
			if (run <= 32)
				encoded[encodedOffset++] = 64 + run - 1;
			else {
				run -= 33;
				encoded[encodedOffset++] = 96 + (run >> 8);
				encoded[encodedOffset++] = run & 255;
			}
			run = 0;
		}
		if (pixel != lastPixel) {
			let indexOffset = (pixel >> 24 ^ pixel >> 16 ^ pixel >> 8 ^ pixel) & 63;
			if (pixel == index[indexOffset])
				encoded[encodedOffset++] = indexOffset;
			else {
				index[indexOffset] = pixel;
				let r = pixel >> 16 & 255;
				let g = pixel >> 8 & 255;
				let b = pixel & 255;
				let a = pixel >> 24 & 255;
				let dr = r - (lastPixel >> 16 & 255);
				let dg = g - (lastPixel >> 8 & 255);
				let db = b - (lastPixel & 255);
				let da = a - (lastPixel >> 24 & 255);
				if (dr >= -16 && dr <= 15 && dg >= -16 && dg <= 15 && db >= -16 && db <= 15 && da >= -16 && da <= 15) {
					if (da == 0 && dr >= -2 && dr <= 1 && dg >= -2 && dg <= 1 && db >= -2 && db <= 1)
						encoded[encodedOffset++] = 170 + (dr << 4) + (dg << 2) + db;
					else if (da == 0 && dg >= -8 && dg <= 7 && db >= -8 && db <= 7) {
						encoded[encodedOffset++] = 208 + dr;
						encoded[encodedOffset++] = 136 + (dg << 4) + db;
					}
					else {
						dr += 16;
						encoded[encodedOffset++] = 224 + (dr >> 1);
						db += 16;
						encoded[encodedOffset++] = ((dr & 1) << 7) + ((dg + 16) << 2) + (db >> 3);
						encoded[encodedOffset++] = ((db & 7) << 5) + da + 16;
					}
				}
				else {
					encoded[encodedOffset++] = 240 | (dr != 0 ? 8 : 0) | (dg != 0 ? 4 : 0) | (db != 0 ? 2 : 0) | (da != 0 ? 1 : 0);
					if (dr != 0)
						encoded[encodedOffset++] = r;
					if (dg != 0)
						encoded[encodedOffset++] = g;
					if (db != 0)
						encoded[encodedOffset++] = b;
					if (da != 0)
						encoded[encodedOffset++] = a;
				}
			}
			lastPixel = pixel;
		}
	}
	encoded.fill(0, encodedOffset, encodedOffset + 4);
	this.encoded = encoded;
	this.encodedSize = encodedOffset + 4;
	return true;
}

QOIEncoder.prototype.getEncoded = function()
{
	return this.encoded;
}

QOIEncoder.prototype.getEncodedSize = function()
{
	return this.encodedSize;
}

function QOIDecoder()
{
}

QOIDecoder.prototype.decode = function(encoded, encodedSize)
{
	if (encoded == null || encodedSize < 19 || encoded[0] != 113 || encoded[1] != 111 || encoded[2] != 105 || encoded[3] != 102)
		return false;
	let width = encoded[4] << 24 | encoded[5] << 16 | encoded[6] << 8 | encoded[7];
	let height = encoded[8] << 24 | encoded[9] << 16 | encoded[10] << 8 | encoded[11];
	if (width <= 0 || height <= 0 || height > (2147483647 / width | 0))
		return false;
	let pixelsSize = width * height;
	let pixels = new Int32Array(pixelsSize);
	encodedSize -= 4;
	let encodedOffset = 14;
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
				let run;
				if (e < 96)
					run = e - 63;
				else
					run = 33 + ((e - 96) << 8) + encoded[encodedOffset++];
				if (pixelsOffset + run > pixelsSize)
					return false;
				pixels.fill(pixel, pixelsOffset, pixelsOffset + run);
				pixelsOffset += run;
			}
			continue;
		}
		else if (e < 224) {
			if (e < 192)
				pixel = (pixel & -16777216) | ((pixel + (((e >> 4) - 8 - 2) << 16)) & 16711680) | ((pixel + (((e >> 2 & 3) - 2) << 8)) & 65280) | ((pixel + (e & 3) - 2) & 255);
			else {
				let d = encoded[encodedOffset++];
				pixel = (pixel & -16777216) | ((pixel + ((e - 192 - 16) << 16)) & 16711680) | ((pixel + (((d >> 4) - 8) << 8)) & 65280) | ((pixel + (d & 15) - 8) & 255);
			}
		}
		else if (e < 240) {
			e = e << 16 | encoded[encodedOffset] << 8 | encoded[encodedOffset + 1];
			encodedOffset += 2;
			pixel = ((pixel + (((e & 31) - 16) << 24)) & -16777216) | ((pixel + (((e >> 15) - 448 - 16) << 16)) & 16711680) | ((pixel + (((e >> 10 & 31) - 16) << 8)) & 65280) | ((pixel + (e >> 5 & 31) - 16) & 255);
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
	this.alpha = encoded[12] == 4;
	this.colorspace = encoded[13];
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

QOIDecoder.prototype.getAlpha = function()
{
	return this.alpha;
}

QOIDecoder.prototype.getColorspace = function()
{
	return this.colorspace;
}
