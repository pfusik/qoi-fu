// Generated automatically with "cito". Do not edit.
#include <stdlib.h>
#include <string.h>
#include "QOI.h"

typedef void (*CiMethodPtr)(void *);
typedef struct {
	size_t count;
	size_t unitSize;
	size_t refCount;
	CiMethodPtr destructor;
} CiShared;

static void *CiShared_Make(size_t count, size_t unitSize, CiMethodPtr constructor, CiMethodPtr destructor)
{
	CiShared *self = (CiShared *) malloc(sizeof(CiShared) + count * unitSize);
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

static void *CiShared_AddRef(void *ptr)
{
	if (ptr != NULL)
		((CiShared *) ptr)[-1].refCount++;
	return ptr;
}

static void CiShared_Release(void *ptr)
{
	if (ptr == NULL)
		return;
	CiShared *self = (CiShared *) ptr - 1;
	if (--self->refCount != 0)
		return;
	if (self->destructor != NULL) {
		for (size_t i = self->count; i > 0;)
			self->destructor((char *) ptr + --i * self->unitSize);
	}
	free(self);
}

static void CiShared_Assign(void **ptr, void *value)
{
	CiShared_Release(*ptr);
	*ptr = value;
}

struct QOIEncoder {
	uint8_t *encoded;
	int encodedSize;
};
static void QOIEncoder_Construct(QOIEncoder *self);
static void QOIEncoder_Destruct(QOIEncoder *self);

struct QOIDecoder {
	int width;
	int height;
	int *pixels;
	bool alpha;
	int colorspace;
};
static void QOIDecoder_Construct(QOIDecoder *self);
static void QOIDecoder_Destruct(QOIDecoder *self);

static void QOIEncoder_Construct(QOIEncoder *self)
{
	self->encoded = NULL;
}

static void QOIEncoder_Destruct(QOIEncoder *self)
{
	CiShared_Release(self->encoded);
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
	return width > 0 && height > 0 && height <= 2147483629 / (width * (alpha ? 5 : 4));
}

bool QOIEncoder_Encode(QOIEncoder *self, int width, int height, int const *pixels, bool alpha, int colorspace)
{
	if (pixels == NULL || !QOIEncoder_CanEncode(width, height, alpha))
		return false;
	int pixelsSize = width * height;
	uint8_t *encoded = (uint8_t *) CiShared_Make(14 + pixelsSize * (alpha ? 5 : 4) + 4, sizeof(uint8_t), NULL, NULL);
	encoded[0] = 113;
	encoded[1] = 111;
	encoded[2] = 105;
	encoded[3] = 102;
	encoded[4] = (uint8_t) (width >> 24);
	encoded[5] = (uint8_t) (width >> 16);
	encoded[6] = (uint8_t) (width >> 8);
	encoded[7] = (uint8_t) width;
	encoded[8] = (uint8_t) (height >> 24);
	encoded[9] = (uint8_t) (height >> 16);
	encoded[10] = (uint8_t) (height >> 8);
	encoded[11] = (uint8_t) height;
	encoded[12] = (uint8_t) (alpha ? 4 : 3);
	encoded[13] = (uint8_t) colorspace;
	int index[64] = { 0 };
	int encodedOffset = 14;
	int lastPixel = -16777216;
	int run = 0;
	for (int pixelsOffset = 0; pixelsOffset < pixelsSize;) {
		int pixel = pixels[pixelsOffset++];
		if (!alpha)
			pixel |= -16777216;
		if (pixel == lastPixel)
			run++;
		if (run > 0 && (pixel != lastPixel || run == 8224 || pixelsOffset >= pixelsSize)) {
			if (run <= 32)
				encoded[encodedOffset++] = (uint8_t) (64 + run - 1);
			else {
				run -= 33;
				encoded[encodedOffset++] = (uint8_t) (96 + (run >> 8));
				encoded[encodedOffset++] = (uint8_t) run;
			}
			run = 0;
		}
		if (pixel != lastPixel) {
			int indexOffset = (pixel >> 24 ^ pixel >> 16 ^ pixel >> 8 ^ pixel) & 63;
			if (pixel == index[indexOffset])
				encoded[encodedOffset++] = (uint8_t) indexOffset;
			else {
				index[indexOffset] = pixel;
				int r = pixel >> 16 & 255;
				int g = pixel >> 8 & 255;
				int b = pixel & 255;
				int a = pixel >> 24 & 255;
				int dr = r - (lastPixel >> 16 & 255);
				int dg = g - (lastPixel >> 8 & 255);
				int db = b - (lastPixel & 255);
				int da = a - (lastPixel >> 24 & 255);
				if (dr >= -16 && dr <= 15 && dg >= -16 && dg <= 15 && db >= -16 && db <= 15 && da >= -16 && da <= 15) {
					if (da == 0 && dr >= -2 && dr <= 1 && dg >= -2 && dg <= 1 && db >= -2 && db <= 1)
						encoded[encodedOffset++] = (uint8_t) (170 + (dr << 4) + (dg << 2) + db);
					else if (da == 0 && dg >= -8 && dg <= 7 && db >= -8 && db <= 7) {
						encoded[encodedOffset++] = (uint8_t) (208 + dr);
						encoded[encodedOffset++] = (uint8_t) (136 + (dg << 4) + db);
					}
					else {
						dr += 16;
						encoded[encodedOffset++] = (uint8_t) (224 + (dr >> 1));
						db += 16;
						encoded[encodedOffset++] = (uint8_t) (((dr & 1) << 7) + ((dg + 16) << 2) + (db >> 3));
						encoded[encodedOffset++] = (uint8_t) (((db & 7) << 5) + da + 16);
					}
				}
				else {
					encoded[encodedOffset++] = (uint8_t) (240 | (dr != 0 ? 8 : 0) | (dg != 0 ? 4 : 0) | (db != 0 ? 2 : 0) | (da != 0 ? 1 : 0));
					if (dr != 0)
						encoded[encodedOffset++] = (uint8_t) r;
					if (dg != 0)
						encoded[encodedOffset++] = (uint8_t) g;
					if (db != 0)
						encoded[encodedOffset++] = (uint8_t) b;
					if (da != 0)
						encoded[encodedOffset++] = (uint8_t) a;
				}
			}
			lastPixel = pixel;
		}
	}
	memset(encoded + encodedOffset, 0, 4 * sizeof(uint8_t));
	CiShared_Assign((void **) &self->encoded, CiShared_AddRef(encoded));
	self->encodedSize = encodedOffset + 4;
	CiShared_Release(encoded);
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
	CiShared_Release(self->pixels);
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
	if (encoded == NULL || encodedSize < 19 || encoded[0] != 113 || encoded[1] != 111 || encoded[2] != 105 || encoded[3] != 102)
		return false;
	int width = encoded[4] << 24 | encoded[5] << 16 | encoded[6] << 8 | encoded[7];
	int height = encoded[8] << 24 | encoded[9] << 16 | encoded[10] << 8 | encoded[11];
	if (width <= 0 || height <= 0 || height > 2147483647 / width)
		return false;
	int pixelsSize = width * height;
	int *pixels = (int *) CiShared_Make(pixelsSize, sizeof(int), NULL, NULL);
	encodedSize -= 4;
	int encodedOffset = 14;
	int index[64] = { 0 };
	int pixel = -16777216;
	for (int pixelsOffset = 0; pixelsOffset < pixelsSize;) {
		if (encodedOffset >= encodedSize) {
			CiShared_Release(pixels);
			return false;
		}
		int e = encoded[encodedOffset++];
		if (e < 128) {
			if (e < 64)
				pixels[pixelsOffset++] = pixel = index[e];
			else {
				int run;
				if (e < 96)
					run = e - 63;
				else
					run = 33 + ((e - 96) << 8) + encoded[encodedOffset++];
				if (pixelsOffset + run > pixelsSize) {
					CiShared_Release(pixels);
					return false;
				}
				for (size_t _i = 0; _i < run; _i++)
					pixels[pixelsOffset + _i] = pixel;
				pixelsOffset += run;
			}
			continue;
		}
		else if (e < 224) {
			if (e < 192)
				pixel = (pixel & -16777216) | ((pixel + (((e >> 4) - 8 - 2) << 16)) & 16711680) | ((pixel + (((e >> 2 & 3) - 2) << 8)) & 65280) | ((pixel + (e & 3) - 2) & 255);
			else {
				int d = encoded[encodedOffset++];
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
	if (encodedOffset != encodedSize) {
		CiShared_Release(pixels);
		return false;
	}
	self->width = width;
	self->height = height;
	CiShared_Assign((void **) &self->pixels, CiShared_AddRef(pixels));
	self->alpha = encoded[12] == 4;
	self->colorspace = encoded[13];
	CiShared_Release(pixels);
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

bool QOIDecoder_GetAlpha(const QOIDecoder *self)
{
	return self->alpha;
}

int QOIDecoder_GetColorspace(const QOIDecoder *self)
{
	return self->colorspace;
}
