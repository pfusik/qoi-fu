// Generated automatically with "cito". Do not edit.
#pragma once
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct QOIEncoder QOIEncoder;

QOIEncoder *QOIEncoder_New(void);
void QOIEncoder_Delete(QOIEncoder *self);

#define QOIEncoder_COLORSPACE_SRGB 0

#define QOIEncoder_COLORSPACE_SRGB_LINEAR_ALPHA 1

#define QOIEncoder_COLORSPACE_LINEAR 15

bool QOIEncoder_Encode(QOIEncoder *self, int width, int height, int const *pixels, bool alpha, int colorspace);

uint8_t const *QOIEncoder_GetEncoded(const QOIEncoder *self);

int QOIEncoder_GetEncodedSize(const QOIEncoder *self);

#ifdef __cplusplus
}
#endif
