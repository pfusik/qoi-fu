// qoiimagine.c - QOI Imagine plugin
//
// Copyright (C) 2021-2022 Piotr Fusik
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

#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "ImagPlug.h"
#include "QOI.h"

static BOOL IMAGINEAPI checkFile(IMAGINEPLUGINFILEINFOTABLE *fileInfoTable, IMAGINELOADPARAM *loadParam, int flags)
{
	return loadParam->length > 4 && memcmp(loadParam->buffer, "qoif", 4) == 0;
}

static LPIMAGINEBITMAP IMAGINEAPI loadFile(IMAGINEPLUGINFILEINFOTABLE *fileInfoTable, IMAGINELOADPARAM *loadParam, int flags)
{
	const IMAGINEPLUGININTERFACE *iface = fileInfoTable->iface;
	if (iface == NULL)
		return NULL;

	QOIDecoder *qoi = QOIDecoder_New();
	if (qoi == NULL) {
		loadParam->errorCode = IMAGINEERROR_OUTOFMEMORY;
		return NULL;
	}
	if (!QOIDecoder_Decode(qoi, loadParam->buffer, loadParam->length)) {
		QOIDecoder_Delete(qoi);
		loadParam->errorCode = IMAGINEERROR_UNSUPPORTEDTYPE;
		return NULL;
	}
	int width = QOIDecoder_GetWidth(qoi);
	int height = QOIDecoder_GetHeight(qoi);
	const int *pixels = QOIDecoder_GetPixels(qoi);
	bool alpha = QOIDecoder_HasAlpha(qoi);

	LPIMAGINEBITMAP bitmap = iface->lpVtbl->Create(width, height, alpha ? 32 : 24, flags);
	if (bitmap == NULL) {
		QOIDecoder_Delete(qoi);
		loadParam->errorCode = IMAGINEERROR_OUTOFMEMORY;
		return NULL;
	}
	if (alpha)
		iface->lpVtbl->SetTransMethod(bitmap, IMAGINETRANSMETHOD_ALPHABLEND);
	if ((flags & IMAGINELOADPARAM_GETINFO) != 0) {
		QOIDecoder_Delete(qoi);
		return bitmap;
	}

	IMAGINECALLBACKPARAM param;
	param.dib = bitmap;
	param.param = loadParam->callback.param;
	param.overall = height - 1;
	param.message = NULL;
	for (int y = 0; y < height; y++) {
		LPBYTE dest = (LPBYTE) iface->lpVtbl->GetLineBits(bitmap, y);
		if (alpha)
			memcpy(dest, pixels + y * width, width << 2);
		else {
			for (int x = 0; x < width; x++) {
				int rgb = *pixels++;
				// 0xRRGGBB -> 0xBB 0xGG 0xRR
				*dest++ = (BYTE) rgb;
				*dest++ = (BYTE) (rgb >> 8);
				*dest++ = (BYTE) (rgb >> 16);
			}
		}
		if ((flags & IMAGINELOADPARAM_CALLBACK) != 0) {
			param.current = y;
			if (!loadParam->callback.proc(&param)) {
				QOIDecoder_Delete(qoi);
				loadParam->errorCode = IMAGINEERROR_ABORTED;
				return bitmap;
			}
		}
	}
	QOIDecoder_Delete(qoi);
	return bitmap;
}

static BOOL IMAGINEAPI saveFile(IMAGINEPLUGINFILEINFOTABLE *fileInfoTable, LPIMAGINEBITMAP bitmap, IMAGINESAVEPARAM *saveParam, int flags)
{
	const IMAGINEPLUGININTERFACE *iface = fileInfoTable->iface;
	if (iface == NULL)
		return FALSE;

	bool alpha;
	switch (iface->lpVtbl->GetBitCount(bitmap)) {
	case 24:
		alpha = false;
		break;
	case 32:
		alpha = true;
		break;
	default:
		saveParam->errorCode = IMAGINEERROR_COLORNOTSUPPORTED;
		return FALSE;
	}
	int width = iface->lpVtbl->GetWidth(bitmap);
	int height = iface->lpVtbl->GetHeight(bitmap);
	if (!QOIEncoder_CanEncode(width, height, alpha)) {
		saveParam->errorCode = IMAGINEERROR_SIZENOTSUPPORTED;
		return FALSE;
	}
	if ((flags & IMAGINESAVEPARAM_TEST) != 0)
		return TRUE;

	int *pixels = (int *) malloc(width * height * sizeof(int));
	if (pixels == NULL) {
		saveParam->errorCode = IMAGINEERROR_OUTOFMEMORY;
		return FALSE;
	}
	for (int y = 0; y < height; y++) {
		LPCBYTE src = (LPCBYTE) iface->lpVtbl->GetLineBits(bitmap, y);
		if (alpha)
			memcpy(pixels + y * width, src, width << 2);
		else {
			for (int x = 0; x < width; x++) {
				// 0xBB 0xGG 0xRR -> 0xRRGGBB
				pixels[y * width + x] = src[2] << 16 | src[1] << 8 | src[0];
				src += 3;
			}
		}
	}

	QOIEncoder *qoi = QOIEncoder_New();
	if (qoi == NULL) {
		free(pixels);
		saveParam->errorCode = IMAGINEERROR_OUTOFMEMORY;
		return FALSE;
	}
	bool ok = QOIEncoder_Encode(qoi, width, height, pixels, alpha, false);
	free(pixels);
	if (!ok) {
		QOIEncoder_Delete(qoi);
		saveParam->errorCode = IMAGINEERROR_SIZENOTSUPPORTED;
		return FALSE;
	}
	int encodedSize = QOIEncoder_GetEncodedSize(qoi);

	saveParam->sb = iface->lpVtbl->sbAlloc(encodedSize, 0);
	if (saveParam->sb == NULL) {
		QOIEncoder_Delete(qoi);
		saveParam->errorCode = IMAGINEERROR_OUTOFMEMORY;
		return FALSE;
	}
	ok = iface->lpVtbl->sbWrite(saveParam->sb, saveParam->sb->current, QOIEncoder_GetEncoded(qoi), encodedSize) != NULL;
	QOIEncoder_Delete(qoi);
	if (!ok) {
		saveParam->errorCode = IMAGINEERROR_WRITEERROR;
		return FALSE;
	}
	return TRUE;
}

static BOOL IMAGINEAPI registerProcW(const IMAGINEPLUGININTERFACE *iface)
{
	static const IMAGINEFILEINFOITEM fileInfoItemW = {
		checkFile,
		loadFile,
		saveFile,
		(LPCTSTR) L"Quite OK Image (QOI)",
		(LPCTSTR) L"QOI\0"
	};
	return iface->lpVtbl->RegisterFileType(&fileInfoItemW) != NULL;
}

__declspec(dllexport) BOOL IMAGINEAPI ImaginePluginGetInfoW(IMAGINEPLUGININFOW *dest)
{
	static const IMAGINEPLUGININFOW pluginInfoW = {
		sizeof(pluginInfoW),
		registerProcW,
		0x02000000,
		L"QOI Plugin",
		IMAGINEPLUGININTERFACE_VERSION
	};
	*dest = pluginInfoW;
	return TRUE;
}
