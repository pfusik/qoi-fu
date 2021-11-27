// Generated automatically with "cito". Do not edit.

"use strict";

function QOIEncoder()
{
}

QOIEncoder.prototype.encode = function(width, height, pixels, alpha)
{
	if (width <= 0 || width >= 65536 || height <= 0 || height >= 65536 || height > (429496726 / width | 0) || pixels == null)
		return false;
	let pixelsSize = width * height;
	let encoded = new Uint8Array(12 + pixelsSize * (alpha ? 5 : 4) + 4);
	encoded[0] = 113;
	encoded[1] = 111;
	encoded[2] = 105;
	encoded[3] = 102;
	encoded[4] = width >> 8;
	encoded[5] = width & 255;
	encoded[6] = height >> 8;
	encoded[7] = height & 255;
	const index = new Int32Array(64);
	let encodedOffset = 12;
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
				if (dr >= -15 && dr <= 16 && dg >= -15 && dg <= 16 && db >= -15 && db <= 16 && da >= -15 && da <= 16) {
					if (da == 0 && dr >= -1 && dr <= 2 && dg >= -1 && dg <= 2 && db >= -1 && db <= 2)
						encoded[encodedOffset++] = 149 + (dr << 4) + (dg << 2) + db;
					else if (da == 0 && dg >= -7 && dg <= 8 && db >= -7 && db <= 8) {
						encoded[encodedOffset++] = 207 + dr;
						encoded[encodedOffset++] = 119 + (dg << 4) + db;
					}
					else {
						dr += 15;
						encoded[encodedOffset++] = 224 + (dr >> 1);
						db += 15;
						encoded[encodedOffset++] = ((dr & 1) << 7) + ((dg + 15) << 2) + (db >> 3);
						encoded[encodedOffset++] = ((db & 7) << 5) + da + 15;
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
	this.encodedSize = encodedOffset + 4;
	let dataSize = this.encodedSize - 12;
	encoded[8] = dataSize >> 24;
	encoded[9] = dataSize >> 16 & 255;
	encoded[10] = dataSize >> 8 & 255;
	encoded[11] = dataSize & 255;
	this.encoded = encoded;
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
