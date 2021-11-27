// Generated automatically with "cito". Do not edit.
#pragma once
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct QOIDecoder QOIDecoder;

QOIDecoder *QOIDecoder_New(void);
void QOIDecoder_Delete(QOIDecoder *self);

bool QOIDecoder_Decode(QOIDecoder *self, uint8_t const *encoded, int encodedSize);

int QOIDecoder_GetWidth(const QOIDecoder *self);

int QOIDecoder_GetHeight(const QOIDecoder *self);

int const *QOIDecoder_GetPixels(const QOIDecoder *self);

#ifdef __cplusplus
}
#endif
