// io-qoi.c - QOI GdkPixbuf loader
//
// Copyright (C) 2022 Piotr Fusik
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

#define GDK_PIXBUF_ENABLE_BACKEND 1
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "QOI-stdio.h"

static void error_memory(GError **error)
{
	g_set_error_literal(error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_INSUFFICIENT_MEMORY, "Out of memory");
}

static void error_decode(GError **error)
{
	g_set_error_literal(error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_UNKNOWN_TYPE, "Error decoding QOI file");
}

static void error_encode(GError **error)
{
	g_set_error_literal(error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_UNSUPPORTED_OPERATION, "Cannot encode to QOI");
}

static void error_write(GError **error)
{
	g_set_error_literal(error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_FAILED, "Error writing file");
}

static GdkPixbuf *to_pixbuf(QOIDecoder *qoi, GError **error)
{
	bool alpha = QOIDecoder_HasAlpha(qoi);
	int width = QOIDecoder_GetWidth(qoi);
	int height = QOIDecoder_GetHeight(qoi);
	GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, alpha, 8, width, height);
	if (pixbuf == NULL) {
		QOIDecoder_Delete(qoi);
		error_memory(error);
		return NULL;
	}

	const int *qoi_pixels = QOIDecoder_GetPixels(qoi);
	guchar *pixbuf_pixels = gdk_pixbuf_get_pixels(pixbuf);
	int pixbuf_stride = gdk_pixbuf_get_rowstride(pixbuf);
	for (int y = 0; y < height; y++) {
		guchar *dest = pixbuf_pixels + y * pixbuf_stride;
		for (int x = 0; x < width; x++) {
			int argb = *qoi_pixels++;
			*dest++ = (guchar) (argb >> 16);
			*dest++ = (guchar) (argb >> 8);
			*dest++ = (guchar) argb;
			if (alpha)
				*dest++ = (guchar) (argb >> 24);
		}
	}
	QOIDecoder_Delete(qoi);
	return pixbuf;
}

static GdkPixbuf *load(FILE *f, GError **error)
{
	QOIDecoder *qoi = QOIDecoder_LoadStdio(f);
	if (qoi == NULL) {
		error_decode(error);
		return NULL;
	}
	return to_pixbuf(qoi, error);
}

typedef struct {
	uint8_t *content;
	int content_len;
	bool failed;
	GdkPixbufModuleSizeFunc size_func;
	GdkPixbufModulePreparedFunc prepared_func;
	GdkPixbufModuleUpdatedFunc updated_func;
	gpointer user_data;
} Context;

static gpointer begin_load(GdkPixbufModuleSizeFunc size_func, GdkPixbufModulePreparedFunc prepared_func,
                           GdkPixbufModuleUpdatedFunc updated_func, gpointer user_data, GError **error)
{
	Context *context = malloc(sizeof(Context));
	if (context != NULL) {
		context->content = NULL;
		context->content_len = 0;
		context->failed = false;
		context->size_func = size_func;
		context->prepared_func = prepared_func;
		context->updated_func = updated_func;
		context->user_data = user_data;
	}
	return context;
}

static gboolean stop_load(gpointer data, GError **error)
{
	Context *context = data;

	if (context->failed) {
		free(context->content);
		free(context);
		return FALSE;
	}

	QOIDecoder *qoi = QOIDecoder_New();
	if (qoi == NULL) {
		free(context->content);
		free(context);
		error_memory(error);
		return FALSE;
	}
	if (!QOIDecoder_Decode(qoi, context->content, context->content_len)) {
		QOIDecoder_Delete(qoi);
		free(context->content);
		free(context);
		error_decode(error);
		return FALSE;
	}
	free(context->content);

	int width = QOIDecoder_GetWidth(qoi);
	int height = QOIDecoder_GetHeight(qoi);
	context->size_func(&width, &height, context->user_data);
	if (width == 0 || height == 0) {
		QOIDecoder_Delete(qoi);
		free(context);
		return TRUE;
	}

	GdkPixbuf *pixbuf = to_pixbuf(qoi, error);
	if (pixbuf == NULL) {
		free(context);
		return FALSE;
	}
	context->prepared_func(pixbuf, NULL, context->user_data);
	context->updated_func(pixbuf, 0, 0, gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf), context->user_data);
	free(context);
	return TRUE;
}

static gboolean load_increment(gpointer data, const guchar *buf, guint size, GError **error)
{
	Context *context = data;

	size_t new_len = (size_t) context->content_len + size;
	if (new_len > INT_MAX) {
		context->failed = true;
		error_decode(error);
		return FALSE;
	}
	context->content = realloc(context->content, new_len);
	if (context->content == NULL) {
		context->failed = true;
		error_memory(error);
		return FALSE;
	}
	memcpy(context->content + context->content_len, buf, size);
	context->content_len = new_len;
	return TRUE;
}

static QOIEncoder *to_qoi(const GdkPixbuf *pixbuf, GError **error)
{
	bool alpha = gdk_pixbuf_get_has_alpha(pixbuf);
	int width = gdk_pixbuf_get_width(pixbuf);
	int height = gdk_pixbuf_get_height(pixbuf);
	if (gdk_pixbuf_get_colorspace(pixbuf) != GDK_COLORSPACE_RGB
	 || gdk_pixbuf_get_bits_per_sample(pixbuf) != 8
	 || !QOIEncoder_CanEncode(width, height, alpha)) {
		error_encode(error);
		return NULL;
	}

	int *qoi_pixels = malloc((size_t) width * height * sizeof(int));
	if (qoi_pixels == NULL) {
		error_memory(error);
		return NULL;
	}
	const guint8 *pixbuf_pixels = gdk_pixbuf_read_pixels(pixbuf);
	int pixbuf_stride = gdk_pixbuf_get_rowstride(pixbuf);
	for (int y = 0; y < height; y++) {
		const guint8 *src = pixbuf_pixels + y * pixbuf_stride;
		for (int x = 0; x < width; x++) {
			int rgb = src[0] << 16 | src[1] << 8 | src[2];
			src += 3;
			qoi_pixels[y * width + x] = (alpha ? *src++ : 0xff) << 24 | rgb;
		}
	}

	QOIEncoder *qoi = QOIEncoder_New();
	if (qoi == NULL) {
		free(qoi_pixels);
		error_memory(error);
		return NULL;
	}
	if (!QOIEncoder_Encode(qoi, width, height, qoi_pixels, alpha, false)) {
		QOIEncoder_Delete(qoi);
		free(qoi_pixels);
		error_encode(error);
		return NULL;
	}
	free(qoi_pixels);
	return qoi;
}

static gboolean save(FILE *f, GdkPixbuf *pixbuf, gchar **param_keys, gchar **param_values, GError **error)
{
	QOIEncoder *qoi = to_qoi(pixbuf, error);
	if (qoi == NULL)
		return FALSE;
	if (!QOIEncoder_SaveStdio(qoi, f)) {
		QOIEncoder_Delete(qoi);
		error_write(error);
		return FALSE;
	}
	QOIEncoder_Delete(qoi);
	return TRUE;
}

static gboolean save_to_callback(GdkPixbufSaveFunc save_func, gpointer user_data, GdkPixbuf *pixbuf, gchar **option_keys, gchar **option_values, GError **error)
{
	QOIEncoder *qoi = to_qoi(pixbuf, error);
	if (qoi == NULL)
		return FALSE;
	if (!save_func((const gchar *) QOIEncoder_GetEncoded(qoi), QOIEncoder_GetEncodedSize(qoi), error, user_data)) {
		QOIEncoder_Delete(qoi);
		error_write(error);
		return FALSE;
	}
	QOIEncoder_Delete(qoi);
	return TRUE;
}

G_MODULE_EXPORT void fill_vtable(GdkPixbufModule *module)
{
	module->load = load;
	module->begin_load = begin_load;
	module->stop_load = stop_load;
	module->load_increment = load_increment;
	module->save = save;
	module->save_to_callback = save_to_callback;
}

G_MODULE_EXPORT void fill_info(GdkPixbufFormat *info)
{
	static GdkPixbufModulePattern signature[] = { { "qoif", NULL, 100 }, { NULL, NULL, 0 } };
	static gchar *mime_types[] = { "image/qoi", NULL };
	static gchar *extensions[] = { "qoi", NULL };
	info->name = "qoi";
	info->signature = signature;
	info->description = "QOI image";
	info->mime_types = mime_types;
	info->extensions = extensions;
	info->flags = GDK_PIXBUF_FORMAT_WRITABLE | GDK_PIXBUF_FORMAT_THREADSAFE;
	info->license = "MIT";
}
