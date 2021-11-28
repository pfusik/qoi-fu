// Generated automatically with "cito". Do not edit.
#pragma once
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct QOIEncoder QOIEncoder;
typedef struct QOIDecoder QOIDecoder;

QOIEncoder *QOIEncoder_New(void);
void QOIEncoder_Delete(QOIEncoder *self);

#define QOIEncoder_COLORSPACE_SRGB 0

#define QOIEncoder_COLORSPACE_SRGB_LINEAR_ALPHA 1

#define QOIEncoder_COLORSPACE_LINEAR 15

bool QOIEncoder_Encode(QOIEncoder *self, int width, int height, int const *pixels, bool alpha, int colorspace);

uint8_t const *QOIEncoder_GetEncoded(const QOIEncoder *self);

int QOIEncoder_GetEncodedSize(const QOIEncoder *self);

QOIDecoder *QOIDecoder_New(void);
void QOIDecoder_Delete(QOIDecoder *self);

bool QOIDecoder_Decode(QOIDecoder *self, uint8_t const *encoded, int encodedSize);

int QOIDecoder_GetWidth(const QOIDecoder *self);

int QOIDecoder_GetHeight(const QOIDecoder *self);

int const *QOIDecoder_GetPixels(const QOIDecoder *self);

bool QOIDecoder_GetAlpha(const QOIDecoder *self);

int QOIDecoder_GetColorspace(const QOIDecoder *self);

#ifdef __cplusplus
}
#endif
