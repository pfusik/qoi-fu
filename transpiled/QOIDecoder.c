// Generated automatically with "cito". Do not edit.
#include <stdlib.h>
#include "QOIDecoder.h"

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

struct QOIDecoder {
	int width;
	int height;
	int *pixels;
	bool alpha;
	int colorspace;
};
static void QOIDecoder_Construct(QOIDecoder *self);
static void QOIDecoder_Destruct(QOIDecoder *self);

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
				int run = 1 + (e & 31);
				if (e >= 96)
					run += 32 + encoded[encodedOffset++];
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
