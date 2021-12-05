// png2qoi.c - command-line converter between PNG and QOI formats
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

#include "QOI.h"
#include "QOI-stdio.h"

static void usage(void)
{
	puts(
		"Usage: png2qoi INPUTFILE...\n"
		"INPUTFILE must be in PNG or QOI format");
}

static bool png2qoi(const char *input_file, FILE *f)
{
	if (fseek(f, 0, SEEK_SET) != 0) {
		perror(input_file);
		fclose(f);
		return false;
	}
	png_image png;
	png.opaque = NULL;
	png.version = PNG_IMAGE_VERSION;
	if (png_image_begin_read_from_stdio(&png, f) == 0) {
		fclose(f);
		fprintf(stderr, "png2qoi: %s: %s\n", input_file, png.message);
		return false;
	}
	bool alpha = (png.format & PNG_FORMAT_FLAG_ALPHA) != 0;
	png.format = PNG_FORMAT_BGRA;
	void *pixels = malloc(PNG_IMAGE_SIZE(png));
	if (pixels == NULL) {
		fprintf(stderr, "png2qoi: %s: out of memory\n", input_file);
		return false;
	}
	if (png_image_finish_read(&png, NULL, pixels, 0, NULL) == 0) {
		free(pixels);
		fclose(f);
		fprintf(stderr, "png2qoi: %s: %s\n", input_file, png.message);
		return false;
	}
	fclose(f);

	char output_file[FILENAME_MAX];
	if (snprintf(output_file, sizeof(output_file), "%s.qoi", input_file) >= sizeof(output_file)) {
		fprintf(stderr, "png2qoi: %s: filename too long\n", input_file);
		return false;
	}

	QOIEncoder *qoi = QOIEncoder_New();
	if (!QOIEncoder_Encode(qoi, png.width, png.height, pixels, alpha, QOIColorspace_SRGB)) {
		QOIEncoder_Delete(qoi);
		free(pixels);
		fprintf(stderr, "png2qoi: %s: error encoding\n", input_file);
		return false;
	}
	free(pixels);

	if (!QOIEncoder_SaveFile(qoi, output_file)) {
		perror(output_file);
		QOIEncoder_Delete(qoi);
		return false;
	}
	QOIEncoder_Delete(qoi);
	return true;
}

static bool qoi2png(const char *input_file, FILE *f)
{
	QOIDecoder *qoi = QOIDecoder_LoadStdio(f);
	if (qoi == NULL) {
		fprintf(stderr, "png2qoi: %s: error loading\n", input_file);
		return false;
	}

	char output_file[FILENAME_MAX];
	if (snprintf(output_file, sizeof(output_file), "%s.png", input_file) >= sizeof(output_file)) {
		fprintf(stderr, "png2qoi: %s: filename too long\n", input_file);
		return false;
	}

	png_image png = { NULL };
	png.version = PNG_IMAGE_VERSION;
	png.width = QOIDecoder_GetWidth(qoi);
	png.height = QOIDecoder_GetHeight(qoi);
	png.format = PNG_FORMAT_BGRA;
	if (png_image_write_to_file(&png, output_file, 0, QOIDecoder_GetPixels(qoi), 0, NULL) == 0) {
		QOIDecoder_Delete(qoi);
		fprintf(stderr, "png2qoi: %s: error encoding\n", output_file);
		return false;
	}
	QOIDecoder_Delete(qoi);
	return true;
}

static bool process_file(const char *input_file)
{
	FILE *f = fopen(input_file, "rb");
	if (f == NULL) {
		perror(input_file);
		return false;
	}
	uint8_t magic[8];
	if (fread(magic, 1, sizeof(magic), f) != sizeof(magic)) {
		fclose(f);
		fprintf(stderr, "png2qoi: %s: file too short\n", input_file);
		return false;
	}

	if (png_check_sig(magic, sizeof(magic)))
		return png2qoi(input_file, f);
	if (memcmp(magic, "qoif", 4) == 0)
		return qoi2png(input_file, f);

	fclose(f);
	fprintf(stderr, "png2qoi: %s: unrecognized file format\n", input_file);
	return f;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		usage();
		return 1;
	}
	bool ok = true;
	for (int i = 1; i < argc; i++) {
		const char *arg = argv[i];
		if (arg[0] != '-')
			ok &= process_file(arg);
		else if (strcmp(arg, "--help") == 0)
			usage();
		else {
			fprintf(stderr, "png2qoi: unknown option: %s\n", arg);
			return 1;
		}
	}
	return ok ? 0 : 1;
}
