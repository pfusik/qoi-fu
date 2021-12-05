// Xqoi.c - QOI XnView plugin
//
// Copyright (C) 2021 Piotr Fusik
//
// MIT License:
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifdef WIN32
#define _WIN32_WINNT 0x0500
#include <windows.h>

#define API __stdcall
#define DLL_EXPORT __declspec(dllexport)

#else

#include <stdint.h>

#define BOOL int32_t
#define DWORD uint32_t
#define INT int32_t

#define API
#define DLL_EXPORT

#define TRUE 1
#define FALSE 0

#endif

#include <stddef.h>
#include <stdlib.h>

#include "QOI-stdio.h"

#define GFP_RGB	0
#define GFP_BGR	1

#define GFP_READ 0x0001
#define GFP_WRITE 0x0002

typedef struct {
	unsigned char red[256];
	unsigned char green[256];
	unsigned char blue[256];
} GFP_COLORMAP;

static size_t strlcpy(char *dst, const char *src, size_t size)
{
	int i;
	for (i = 0; i < size - 1 && src[i] != '\0'; i++)
		dst[i] = src[i];
	dst[i] = '\0';
	while (src[i] != '\0')
		i++;
	return i;
}

DLL_EXPORT BOOL API gfpGetPluginInfo(
	DWORD version, char *label, INT label_max_size,
	char *extension, INT extension_max_size, INT *support)
{
	if (version != 0x0002)
		return FALSE;

	strlcpy(label, "Quite OK Image", label_max_size);
	strlcpy(extension, "qoi", extension_max_size);
	*support = GFP_READ | GFP_WRITE;

	return TRUE;
}

DLL_EXPORT void * API gfpLoadPictureInit(const char *filename)
{
	return QOIDecoder_LoadFile(filename);
}

DLL_EXPORT BOOL API gfpLoadPictureGetInfo(
	void *ptr, INT *pictype, INT *width, INT *height,
	INT *dpi, INT *bits_per_pixel, INT *bytes_per_line,
	BOOL *has_colormap, char *label, INT label_max_size)
{
	const QOIDecoder *qoi = (const QOIDecoder *) ptr;

	*pictype = GFP_RGB;
	*width = QOIDecoder_GetWidth(qoi);
	*height = QOIDecoder_GetHeight(qoi);
	*dpi = 96;
	int bytes_per_pixel = QOIDecoder_GetAlpha(qoi) ? 4 : 3;
	*bits_per_pixel = bytes_per_pixel << 3;
	*bytes_per_line = *width * bytes_per_pixel;
	*has_colormap = FALSE;
	strlcpy(label, "Quite OK Image", label_max_size);

	return TRUE;
}

DLL_EXPORT BOOL API gfpLoadPictureGetLine(void *ptr, INT line, unsigned char *buffer)
{
	const QOIDecoder *qoi = (const QOIDecoder *) ptr;
	int width = QOIDecoder_GetWidth(qoi);
	const int *pixels = QOIDecoder_GetPixels(qoi) + line * width;
	int bytes_per_pixel = QOIDecoder_GetAlpha(qoi) ? 4 : 3;

	for (int x = 0; x < width; x++) {
		int rgb = pixels[x];
		buffer[x * bytes_per_pixel] = (unsigned char) (rgb >> 16);
		buffer[x * bytes_per_pixel + 1] = (unsigned char) (rgb >> 8);
		buffer[x * bytes_per_pixel + 2] = (unsigned char) rgb;
		if (bytes_per_pixel == 4)
			buffer[x * bytes_per_pixel + 3] = (unsigned char) (rgb >> 24);
	}

	return TRUE;
}

DLL_EXPORT BOOL API gfpLoadPictureGetColormap(void *ptr, GFP_COLORMAP *cmap)
{
	return FALSE;
}

DLL_EXPORT void API gfpLoadPictureExit(void *ptr)
{
	QOIDecoder *qoi = (QOIDecoder *) ptr;
	QOIDecoder_Delete(qoi);
}

DLL_EXPORT BOOL API gfpSavePictureIsSupported(INT width, INT height, INT bits_per_pixel, BOOL has_colormap)
{
	return (bits_per_pixel == 24 || bits_per_pixel == 32)
		&& !has_colormap
		&& QOIEncoder_CanEncode(width, height, bits_per_pixel == 32);
}

typedef struct {
	int width;
	int height;
	bool alpha;
	FILE *f;
	int pixels[];
} QOIWriter;

DLL_EXPORT void * API gfpSavePictureInit(
	const char *filename, INT width, INT height, INT bits_per_pixel,
	INT dpi, INT *picture_type, char *label, INT label_max_size)
{
	FILE *f = fopen(filename, "wb");
	if (f == NULL)
		return NULL;
	QOIWriter *w = (QOIWriter *) malloc(sizeof(QOIWriter) + width * height * sizeof(int));
	if (w == NULL) {
		fclose(f);
		return NULL;
	}
	w->width = width;
	w->height = height;
	w->alpha = bits_per_pixel == 32;
	w->f = f;
	*picture_type = GFP_RGB;
	strlcpy(label, "Quite OK Image", label_max_size);
	return w;
}

DLL_EXPORT BOOL API gfpSavePicturePutLine(void *ptr, INT line, const unsigned char *buffer)
{
	QOIWriter *w = (QOIWriter *) ptr;
	int width = w->width;
	bool alpha = w->alpha;
	int bytes_per_pixel = alpha ? 4 : 3;
	int *pixels = w->pixels + line * width;
	for (int x = 0; x < width; x++) {
		pixels[x] = buffer[x * bytes_per_pixel] << 16
			| buffer[x * bytes_per_pixel + 1] << 8
			| buffer[x * bytes_per_pixel + 2]
			| (alpha ? buffer[x * bytes_per_pixel + 3] : 0xff) << 24;
	}
	return TRUE;
}

DLL_EXPORT void API gfpSavePictureExit(void *ptr)
{
	QOIWriter *w = (QOIWriter *) ptr;
	QOIEncoder *qoi = QOIEncoder_New();
	if (QOIEncoder_Encode(qoi, w->width, w->height, w->pixels, w->alpha, QOIColorspace_SRGB))
		QOIEncoder_SaveStdio(qoi, w->f);
	else
		fclose(w->f);
	QOIEncoder_Delete(qoi);
	free(w);
}

#ifdef WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return TRUE;
}
#endif
