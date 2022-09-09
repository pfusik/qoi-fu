// qoi-stdio.c - QOI format I/O using stdio.h
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

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "QOI-stdio.h"

bool QOIEncoder_SaveStdio(const QOIEncoder *qoi, FILE *f)
{
	int size = QOIEncoder_GetEncodedSize(qoi);
	return fwrite(QOIEncoder_GetEncoded(qoi), 1, size, f) == size;
}

bool QOIEncoder_SaveFile(const QOIEncoder *qoi, const char *filename)
{
	FILE *f = fopen(filename, "wb");
	if (f == NULL)
		return false;
	bool saveResult = QOIEncoder_SaveStdio(qoi, f);
	int closeResult = fclose(f);
	return saveResult && closeResult == 0;
}

QOIDecoder *QOIDecoder_LoadStdio(FILE *f)
{
	long encoded_size;
	if (fseek(f, 0, SEEK_END) != 0
	 || (encoded_size = ftell(f)) < 0
	 || fseek(f, 0, SEEK_SET) != 0
	 || encoded_size > INT_MAX)
		return NULL;
	void *encoded = malloc(encoded_size);
	if (encoded == NULL
	 || fread(encoded, 1, encoded_size, f) != encoded_size)
		return NULL;

	QOIDecoder *qoi = QOIDecoder_New();
	if (!QOIDecoder_Decode(qoi, (const uint8_t *) encoded, (int) encoded_size)) {
		QOIDecoder_Delete(qoi);
		free(encoded);
		return NULL;
	}
	free(encoded);
	return qoi;
}

QOIDecoder *QOIDecoder_LoadFile(const char *filename)
{
	FILE *f = fopen(filename, "rb");
	if (f == NULL)
		return NULL;
	QOIDecoder *qoi = QOIDecoder_LoadStdio(f);
	fclose(f);
	return qoi;
}
