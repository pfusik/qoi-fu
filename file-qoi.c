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
#include <libgimp/gimpui.h>

#include "QOI-stdio.h"

#define AUTHOR "Piotr Fusik <fox@scene.pl>"
#define DATE "2021"
#define PLUG_IN_BINARY "file-qoi"
#define LOAD_PROC "file-qoi-load"
#define SAVE_PROC "file-qoi-save"

static const Babl *get_format(bool linear)
{
	return linear
		? babl_format_new(babl_model("RGBA"), babl_type("u8"),
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
			babl_component("B"), babl_component("G"), babl_component("R"), babl_component("A"),
#else
			babl_component("A"), babl_component("R"), babl_component("G"), babl_component("B"),
#endif
			NULL)
		: babl_format_new(babl_model("R'G'B'A"), babl_type("u8"),
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
			babl_component("B'"), babl_component("G'"), babl_component("R'"), babl_component("A"),
#else
			babl_component("A"), babl_component("R'"), babl_component("G'"), babl_component("B'"),
#endif
			NULL);
}

static gint32 load_image(const gchar *filename)
{
	QOIDecoder *qoi = QOIDecoder_LoadFile(filename);
	if (qoi == NULL)
		return -1;
	gegl_init(NULL, NULL);
	int width = QOIDecoder_GetWidth(qoi);
	int height = QOIDecoder_GetHeight(qoi);
	bool linear = QOIDecoder_IsLinearColorspace(qoi);
	gint32 image = gimp_image_new_with_precision(width, height, GIMP_RGB,
		linear ? GIMP_PRECISION_U8_LINEAR : GIMP_PRECISION_U8_GAMMA);
	if (image != -1) {
		gimp_image_set_filename(image, filename);
		gint32 layer = gimp_layer_new(image, "Background", width, height,
			QOIDecoder_HasAlpha(qoi) ? GIMP_RGBA_IMAGE : GIMP_RGB_IMAGE, 100, GIMP_NORMAL_MODE);
		gimp_image_insert_layer(image, layer, -1, 0);
		GeglBuffer *buffer = gimp_drawable_get_buffer(layer);
		gegl_buffer_set(buffer, NULL, 0, get_format(linear), QOIDecoder_GetPixels(qoi), GEGL_AUTO_ROWSTRIDE);
		g_object_unref(buffer);
	}
	QOIDecoder_Delete(qoi);
	return image;
}

static bool save_image(const gchar *filename, gint32 image, gint32 drawable)
{
	GeglBuffer *buffer = gimp_drawable_get_buffer(drawable);
	int width = gegl_buffer_get_width(buffer);
	int height = gegl_buffer_get_height(buffer);
	int *pixels = (int *) malloc((size_t) width * height * sizeof(int));
	if (pixels == NULL) {
		g_object_unref(buffer);
		return false;
	}
	const Babl *format = get_format(gimp_image_get_precision(image) == GIMP_PRECISION_U8_LINEAR);
	gegl_buffer_get(buffer, NULL, 1, format, pixels, GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);
	g_object_unref(buffer);
	bool alpha = gimp_drawable_has_alpha(drawable);

	QOIEncoder *qoi = QOIEncoder_New();
	bool ok = QOIEncoder_Encode(qoi, width, height, pixels, alpha, false);
	free(pixels);
	if (ok)
		ok = QOIEncoder_SaveFile(qoi, filename);
	QOIEncoder_Delete(qoi);
	return ok;
}

static void query(void)
{
	static const GimpParamDef load_args[] = {
		{ GIMP_PDB_INT32,  "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
		{ GIMP_PDB_STRING, "filename",     "The name of the file to load" },
		{ GIMP_PDB_STRING, "raw-filename", "The name entered" }
	};
	static const GimpParamDef load_return_vals[] = {
		{ GIMP_PDB_IMAGE, "image", "Output image" }
	};
	gimp_install_procedure(LOAD_PROC,
		"Loads Quite OK Image (QOI) files",
		"Loads Quite OK Image (QOI) files",
		AUTHOR,
		AUTHOR,
		DATE,
		NULL,
		NULL,
		GIMP_PLUGIN,
		G_N_ELEMENTS(load_args),
		G_N_ELEMENTS(load_return_vals),
		load_args,
		load_return_vals);
	gimp_register_load_handler(LOAD_PROC, "qoi", "");

	static const GimpParamDef save_args[] = {
		{ GIMP_PDB_INT32,    "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
		{ GIMP_PDB_IMAGE,    "image",        "Input image" },
		{ GIMP_PDB_DRAWABLE, "drawable",     "Drawable to save" },
		{ GIMP_PDB_STRING,   "filename",     "The name of the file to save the image in" },
		{ GIMP_PDB_STRING,   "raw-filename", "The name entered" }
	};
	gimp_install_procedure(SAVE_PROC,
		"Saves Quite OK Image (QOI) files",
		"Saves Quite OK Image (QOI) files",
		AUTHOR,
		AUTHOR,
		DATE,
		NULL,
		"RGB*",
		GIMP_PLUGIN,
		G_N_ELEMENTS(save_args),
		0,
		save_args,
		NULL);
	gimp_register_save_handler(SAVE_PROC, "qoi", "");
}

static void run(const gchar *name, gint nparams, const GimpParam *param, gint *nreturn_vals, GimpParam **return_vals)
{
	static GimpParam values[2];
	*return_vals = values;
	*nreturn_vals = 1;
	values[0].type = GIMP_PDB_STATUS;
	if (strcmp(name, LOAD_PROC) == 0 && nparams >= 2) {
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
	else if (strcmp(name, SAVE_PROC) == 0 && nparams >= 4) {
		GimpRunMode run_mode = param[0].data.d_int32;
		gint32 image = param[1].data.d_int32;
		gint32 drawable = param[2].data.d_int32;
		GimpExportReturn export = GIMP_EXPORT_CANCEL;
		if (run_mode == GIMP_RUN_INTERACTIVE || run_mode == GIMP_RUN_WITH_LAST_VALS) {
			gimp_ui_init(PLUG_IN_BINARY, FALSE);
			export = gimp_export_image(&image, &drawable, "QOI", GIMP_EXPORT_CAN_HANDLE_RGB | GIMP_EXPORT_CAN_HANDLE_ALPHA);
			if (export == GIMP_EXPORT_CANCEL) {
				values[0].data.d_status = GIMP_PDB_CANCEL;
				return;
			}
		}
		if (save_image(param[3].data.d_string, image, drawable))
			values[0].data.d_status = GIMP_PDB_SUCCESS;
		else
			values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;
		if (export == GIMP_EXPORT_EXPORT)
			gimp_image_delete(image);
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
