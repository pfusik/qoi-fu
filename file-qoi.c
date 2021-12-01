// file-qoi.c - QOI GIMP plugin
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
#include <string.h>
#include <libgimp/gimp.h>

#include "QOI-stdio.h"

#define LOAD_PROC "file-qoi-load"

static gint32 load_image(const gchar *filename)
{
	QOIDecoder *qoi = QOIDecoder_LoadFile(filename);
	if (qoi == NULL)
		return -1;
	gegl_init(NULL, NULL);
	int width = QOIDecoder_GetWidth(qoi);
	int height = QOIDecoder_GetHeight(qoi);
	gint32 image = gimp_image_new(width, height, GIMP_RGB);
	if (image != -1) {
		gimp_image_set_filename(image, filename);
		bool alpha = QOIDecoder_GetAlpha(qoi);
		gint32 layer = gimp_layer_new(image, "Background", width, height, alpha ? GIMP_RGBA_IMAGE : GIMP_RGB_IMAGE, 100, GIMP_NORMAL_MODE);
		gimp_image_insert_layer(image, layer, -1, 0);
		int bpp = alpha ? 4 : 3;
		uint8_t *line = (uint8_t *) malloc(width * bpp);
		if (line != NULL) {
			const int *pixels = QOIDecoder_GetPixels(qoi);
			GeglBuffer *buffer = gimp_drawable_get_buffer(layer);
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					int rgb = pixels[y * width + x];
					line[x * bpp] = (uint8_t) (rgb >> 16);
					line[x * bpp + 1] = (uint8_t) (rgb >> 8);
					line[x * bpp + 2] = (uint8_t) rgb;
					if (alpha)
						line[x * bpp + 3] = (uint8_t) (rgb >> 24);
				}
				gegl_buffer_set(buffer, GEGL_RECTANGLE (0, y, width, 1), 0, NULL, line, GEGL_AUTO_ROWSTRIDE);
			}
			g_object_unref(buffer);
			free(line);
		}
		else {
			gimp_image_delete(image);
			image = -1;
		}
	}
	QOIDecoder_Delete(qoi);
	return image;
}

static void query(void)
{
	static const GimpParamDef load_args[] = {
		{ GIMP_PDB_INT32,  "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
		{ GIMP_PDB_STRING, "filename",     "The name of the file to load" },
		{ GIMP_PDB_STRING, "raw-filename", "The name entered"             }
	};
	static const GimpParamDef load_return_vals[] =
	{
		{ GIMP_PDB_IMAGE, "image", "Output image" }
	};
	gimp_install_procedure(LOAD_PROC,
		"Loads Quite OK Image (QOI) files",
		"Loads Quite OK Image (QOI) files",
		"Piotr Fusik <fox@scene.pl>",
		"Piotr Fusik <fox@scene.pl>",
		"2021",
		NULL,
		NULL,
		GIMP_PLUGIN,
		G_N_ELEMENTS(load_args),
		G_N_ELEMENTS(load_return_vals),
		load_args,
		load_return_vals);
	gimp_register_load_handler(LOAD_PROC, "qoi", "");
}

static void run(const gchar *name, gint nparams, const GimpParam *param, gint *nreturn_vals, GimpParam **return_vals)
{
	static GimpParam values[2];
	*return_vals = values;
	*nreturn_vals = 1;
	values[0].type = GIMP_PDB_STATUS;
	if (strcmp(name, LOAD_PROC) == 0) {
		gint32 image = load_image(param[1].data.d_string);
		if (image != -1) {
			values[0].data.d_status = GIMP_PDB_SUCCESS;
			values[1].type = GIMP_PDB_IMAGE;
			values[1].data.d_image = image;
			*nreturn_vals = 2;
		}
		else
			values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;
	}
	else
		values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
}

const GimpPlugInInfo PLUG_IN_INFO = {
	NULL,
	NULL,
	query,
	run
};

MAIN()
