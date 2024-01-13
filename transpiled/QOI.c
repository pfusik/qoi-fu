// Generated automatically with "fut". Do not edit.
#include <stdlib.h>
#include <string.h>
#include "QOI.h"

typedef void (*FuMethodPtr)(void *);
typedef struct {
	size_t count;
	size_t unitSize;
	size_t refCount;
	FuMethodPtr destructor;
} FuShared;

static void *FuShared_Make(size_t count, size_t unitSize, FuMethodPtr constructor, FuMethodPtr destructor)
{
	FuShared *self = (FuShared *) malloc(sizeof(FuShared) + count * unitSize);
	self->count = count;
	self->unitSize = unitSize;
	self->refCount = 1;
	self->destructor = destructor;
	if (constructor != NULL) {
		for (size_t i = 0; i < count; i++)
			constructor((char *) (self + 1) + i * unitSize);
	}
	return self + 1;
}

static void *FuShared_AddRef(void *ptr)
{
	if (ptr != NULL)
		((FuShared *) ptr)[-1].refCount++;
	return ptr;
}

static void FuShared_Release(void *ptr)
{
	if (ptr == NULL)
		return;
	FuShared *self = (FuShared *) ptr - 1;
	if (--self->refCount != 0)
		return;
	if (self->destructor != NULL) {
		for (size_t i = self->count; i > 0;)
			self->destructor((char *) ptr + --i * self->unitSize);
	}
	free(self);
}

static void FuShared_Assign(void **ptr, void *value)
{
	FuShared_Release(*ptr);
	*ptr = value;
}

/**
 * Encoder of the "Quite OK Image" (QOI) format.
 * Losslessly compresses an image to a byte array.
 */
struct QOIEncoder {
	uint8_t *encoded;
	int encodedSize;
};
static void QOIEncoder_Construct(QOIEncoder *self);
static void QOIEncoder_Destruct(QOIEncoder *self);

/**
 * Decoder of the "Quite OK Image" (QOI) format.
 */
struct QOIDecoder {
	int width;
	int height;
	int *pixels;
	bool alpha;
	bool linearColorspace;
};
static void QOIDecoder_Construct(QOIDecoder *self);
static void QOIDecoder_Destruct(QOIDecoder *self);

static void QOIEncoder_Construct(QOIEncoder *self)
{
	self->encoded = NULL;
}

static void QOIEncoder_Destruct(QOIEncoder *self)
{
	FuShared_Release(self->encoded);
}

QOIEncoder *QOIEncoder_New(void)
{
	QOIEncoder *self = (QOIEncoder *) malloc(sizeof(QOIEncoder));
	if (self != NULL)
		QOIEncoder_Construct(self);
	return self;
}

void QOIEncoder_Delete(QOIEncoder *self)
{
	if (self == NULL)
		return;
	QOIEncoder_Destruct(self);
	free(self);
}

bool QOIEncoder_CanEncode(int width, int height, bool alpha)
{
	return width > 0 && height > 0 && height <= 2147483625 / width / (alpha ? 5 : 4);
}

bool QOIEncoder_Encode(QOIEncoder *self, int width, int height, int const *pixels, bool alpha, bool linearColorspace)
{
	if (!QOIEncoder_CanEncode(width, height, alpha))
		return false;
	int pixelsSize = width * height;
	uint8_t *encoded = (uint8_t *) FuShared_Make(14 + pixelsSize * (alpha ? 5 : 4) + 8, sizeof(uint8_t), NULL, NULL);
	encoded[0] = 'q';
	encoded[1] = 'o';
	encoded[2] = 'i';
	encoded[3] = 'f';
	encoded[4] = (uint8_t) (width >> 24);
	encoded[5] = (uint8_t) (width >> 16);
	encoded[6] = (uint8_t) (width >> 8);
	encoded[7] = (uint8_t) width;
	encoded[8] = (uint8_t) (height >> 24);
	encoded[9] = (uint8_t) (height >> 16);
	encoded[10] = (uint8_t) (height >> 8);
	encoded[11] = (uint8_t) height;
	encoded[12] = (uint8_t) (alpha ? 4 : 3);
	encoded[13] = (uint8_t) (linearColorspace ? 1 : 0);
	int index[64] = { 0 };
	int encodedOffset = 14;
	int lastPixel = -16777216;
	int run = 0;
	for (int pixelsOffset = 0; pixelsOffset < pixelsSize;) {
		int pixel = pixels[pixelsOffset++];
		if (!alpha)
			pixel |= -16777216;
		if (pixel == lastPixel) {
			if (++run == 62 || pixelsOffset >= pixelsSize) {
				encoded[encodedOffset++] = (uint8_t) (191 + run);
				run = 0;
			}
		}
		else {
			if (run > 0) {
				encoded[encodedOffset++] = (uint8_t) (191 + run);
				run = 0;
			}
			int indexOffset = ((pixel >> 16) * 3 + (pixel >> 8) * 5 + (pixel & 63) * 7 + (pixel >> 24) * 11) & 63;
			if (pixel == index[indexOffset])
				encoded[encodedOffset++] = (uint8_t) indexOffset;
			else {
				index[indexOffset] = pixel;
				int r = pixel >> 16 & 255;
				int g = pixel >> 8 & 255;
				int b = pixel & 255;
				if ((pixel ^ lastPixel) >> 24 != 0) {
					encoded[encodedOffset] = 255;
					encoded[encodedOffset + 1] = (uint8_t) r;
					encoded[encodedOffset + 2] = (uint8_t) g;
					encoded[encodedOffset + 3] = (uint8_t) b;
					encoded[encodedOffset + 4] = (uint8_t) (pixel >> 24);
					encodedOffset += 5;
				}
				else {
					int dr = r - (lastPixel >> 16 & 255);
					int dg = g - (lastPixel >> 8 & 255);
					int db = b - (lastPixel & 255);
					if (dr >= -2 && dr <= 1 && dg >= -2 && dg <= 1 && db >= -2 && db <= 1)
						encoded[encodedOffset++] = (uint8_t) (106 + (dr << 4) + (dg << 2) + db);
					else {
						dr -= dg;
						db -= dg;
						if (dr >= -8 && dr <= 7 && dg >= -32 && dg <= 31 && db >= -8 && db <= 7) {
							encoded[encodedOffset] = (uint8_t) (160 + dg);
							encoded[encodedOffset + 1] = (uint8_t) (136 + (dr << 4) + db);
							encodedOffset += 2;
						}
						else {
							encoded[encodedOffset] = 254;
							encoded[encodedOffset + 1] = (uint8_t) r;
							encoded[encodedOffset + 2] = (uint8_t) g;
							encoded[encodedOffset + 3] = (uint8_t) b;
							encodedOffset += 4;
						}
					}
				}
			}
			lastPixel = pixel;
		}
	}
	memset(encoded + encodedOffset, 0, 7 * sizeof(uint8_t));
	encoded[encodedOffset + 8 - 1] = 1;
	FuShared_Assign((void **) &self->encoded, FuShared_AddRef(encoded));
	self->encodedSize = encodedOffset + 8;
	FuShared_Release(encoded);
	return true;
}

uint8_t const *QOIEncoder_GetEncoded(const QOIEncoder *self)
{
	return self->encoded;
}

int QOIEncoder_GetEncodedSize(const QOIEncoder *self)
{
	return self->encodedSize;
}

static void QOIDecoder_Construct(QOIDecoder *self)
{
	self->pixels = NULL;
}

static void QOIDecoder_Destruct(QOIDecoder *self)
{
	FuShared_Release(self->pixels);
}

QOIDecoder *QOIDecoder_New(void)
{
	QOIDecoder *self = (QOIDecoder *) malloc(sizeof(QOIDecoder));
	if (self != NULL)
		QOIDecoder_Construct(self);
	return self;
}

void QOIDecoder_Delete(QOIDecoder *self)
{
	if (self == NULL)
		return;
	QOIDecoder_Destruct(self);
	free(self);
}

bool QOIDecoder_Decode(QOIDecoder *self, uint8_t const *encoded, int encodedSize)
{
	if (encodedSize < 23 || encoded[0] != 'q' || encoded[1] != 'o' || encoded[2] != 'i' || encoded[3] != 'f')
		return false;
	int width = encoded[4] << 24 | encoded[5] << 16 | encoded[6] << 8 | encoded[7];
	int height = encoded[8] << 24 | encoded[9] << 16 | encoded[10] << 8 | encoded[11];
	if (width <= 0 || height <= 0 || height > 2147483647 / width)
		return false;
	switch (encoded[12]) {
	case 3:
		self->alpha = false;
		break;
	case 4:
		self->alpha = true;
		break;
	default:
		return false;
	}
	switch (encoded[13]) {
	case 0:
		self->linearColorspace = false;
		break;
	case 1:
		self->linearColorspace = true;
		break;
	default:
		return false;
	}
	int pixelsSize = width * height;
	int *pixels = (int *) FuShared_Make(pixelsSize, sizeof(int), NULL, NULL);
	encodedSize -= 8;
	int encodedOffset = 14;
	int index[64] = { 0 };
	int pixel = -16777216;
	for (int pixelsOffset = 0; pixelsOffset < pixelsSize;) {
		if (encodedOffset >= encodedSize) {
			FuShared_Release(pixels);
			return false;
		}
		int e = encoded[encodedOffset++];
		switch (e >> 6) {
		case 0:
			pixels[pixelsOffset++] = pixel = index[e];
			continue;
		case 1:
			pixel = (pixel & -16777216) | ((pixel + (((e >> 4) - 4 - 2) << 16)) & 16711680) | ((pixel + (((e >> 2 & 3) - 2) << 8)) & 65280) | ((pixel + (e & 3) - 2) & 255);
			break;
		case 2:
			e -= 160;
			int rb = encoded[encodedOffset++];
			pixel = (pixel & -16777216) | ((pixel + ((e + (rb >> 4) - 8) << 16)) & 16711680) | ((pixel + (e << 8)) & 65280) | ((pixel + e + (rb & 15) - 8) & 255);
			break;
		default:
			if (e < 254) {
				e -= 191;
				if (pixelsOffset + e > pixelsSize) {
					FuShared_Release(pixels);
					return false;
				}
				for (int _i = 0; _i < e; _i++)
					pixels[pixelsOffset + _i] = pixel;
				pixelsOffset += e;
				continue;
			}
			if (e == 254) {
				pixel = (pixel & -16777216) | encoded[encodedOffset] << 16 | encoded[encodedOffset + 1] << 8 | encoded[encodedOffset + 2];
				encodedOffset += 3;
			}
			else {
				pixel = encoded[encodedOffset + 3] << 24 | encoded[encodedOffset] << 16 | encoded[encodedOffset + 1] << 8 | encoded[encodedOffset + 2];
				encodedOffset += 4;
			}
			break;
		}
		pixels[pixelsOffset++] = index[((pixel >> 16) * 3 + (pixel >> 8) * 5 + (pixel & 63) * 7 + (pixel >> 24) * 11) & 63] = pixel;
	}
	if (encodedOffset != encodedSize) {
		FuShared_Release(pixels);
		return false;
	}
	self->width = width;
	self->height = height;
	FuShared_Assign((void **) &self->pixels, FuShared_AddRef(pixels));
	FuShared_Release(pixels);
	return true;
}

int QOIDecoder_GetWidth(const QOIDecoder *self)
{
	return self->width;
}

int QOIDecoder_GetHeight(const QOIDecoder *self)
{
	return self->height;
}

int const *QOIDecoder_GetPixels(const QOIDecoder *self)
{
	return self->pixels;
}

bool QOIDecoder_HasAlpha(const QOIDecoder *self)
{
	return self->alpha;
}

bool QOIDecoder_IsLinearColorspace(const QOIDecoder *self)
{
	return self->linearColorspace;
}
