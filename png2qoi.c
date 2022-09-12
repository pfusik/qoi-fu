// png2qoi.c - command-line converter between PNG and QOI formats
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
		"Usage: png2qoi [OPTIONS] INPUTFILE...\n"
		"INPUTFILE must be in PNG or QOI format.\n"
		"Options:\n"
		"-o FILE  --output=FILE   Set output file name\n"
		"-h       --help          Display this information\n"
		"-v       --version       Display version information"
	);
}

static bool png2qoi(const char *input_file, FILE *f, const char *output_file)
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

	QOIEncoder *qoi = QOIEncoder_New();
	if (!QOIEncoder_Encode(qoi, png.width, png.height, pixels, alpha, false)) {
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

static bool qoi2png(const char *input_file, FILE *f, const char *output_file)
{
	QOIDecoder *qoi = QOIDecoder_LoadStdio(f);
	fclose(f);
	if (qoi == NULL) {
		fprintf(stderr, "png2qoi: %s: error loading\n", input_file);
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

static bool process_file(const char *input_file, const char *output_file)
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

	char default_output_file[FILENAME_MAX];
	if (output_file == NULL) {
		const char *ext = png_check_sig(magic, sizeof(magic)) ? "qoi" : "png";
		if (snprintf(default_output_file, sizeof(default_output_file), "%s.%s", input_file, ext) >= sizeof(default_output_file)) {
			fclose(f);
			fprintf(stderr, "png2qoi: %s: filename too long\n", input_file);
			return false;
		}
		output_file = default_output_file;
	}

	if (png_check_sig(magic, sizeof(magic)))
		return png2qoi(input_file, f, output_file);
	if (memcmp(magic, "qoif", 4) == 0)
		return qoi2png(input_file, f, output_file);

	fclose(f);
	fprintf(stderr, "png2qoi: %s: unrecognized file format\n", input_file);
	return false;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		usage();
		return 1;
	}
	const char *output_file = NULL;
	bool ok = true;
	for (int i = 1; i < argc; i++) {
		const char *arg = argv[i];
		if (arg[0] != '-') {
			ok &= process_file(arg, output_file);
			output_file = NULL;
		}
		else if (arg[1] == 'o' && arg[2] == '\0' && i + 1 < argc)
			output_file = argv[++i];
		else if (strncmp(arg, "--output=", 9) == 0)
			output_file = arg + 9;
		else if ((arg[1] == 'h' && arg[2] == '\0') || strcmp(arg, "--help") == 0)
			usage();
		else if ((arg[1] == 'v' && arg[2] == '\0') || strcmp(arg, "--version") == 0)
			puts("png2qoi 2.0.0");
		else {
			fprintf(stderr, "png2qoi: unknown option: %s\n", arg);
			return 1;
		}
	}
	return ok ? 0 : 1;
}
