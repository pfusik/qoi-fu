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

/**
 * Determines if an image of given size can be encoded.
 * @param width Image width in pixels.
 * @param height Image height in pixels.
 * @param alpha Whether the image has the alpha channel (transparency).
 */
bool QOIEncoder_CanEncode(int width, int height, bool alpha);

/**
 * Encodes the given image.
 * Returns <code>true</code> if encoded successfully.
 * @param self This <code>QOIEncoder</code>.
 * @param width Image width in pixels.
 * @param height Image height in pixels.
 * @param pixels Pixels of the image, top-down, left-to-right.
 * @param alpha <code>false</code> specifies that all pixels are opaque. High bytes of <code>pixels</code> elements are ignored then.
 * @param linearColorspace Specifies the color space.
 */
bool QOIEncoder_Encode(QOIEncoder *self, int width, int height, int const *pixels, bool alpha, bool linearColorspace);

/**
 * Returns the encoded file contents.
 * This method can only be called after <code>Encode</code> returned <code>true</code>.
 * The allocated array is usually larger than the encoded data.
 * Call <code>GetEncodedSize</code> to retrieve the number of leading bytes that are significant.
 * @param self This <code>QOIEncoder</code>.
 */
uint8_t const *QOIEncoder_GetEncoded(const QOIEncoder *self);

/**
 * Returns the encoded file length.
 * @param self This <code>QOIEncoder</code>.
 */
int QOIEncoder_GetEncodedSize(const QOIEncoder *self);

QOIDecoder *QOIDecoder_New(void);
void QOIDecoder_Delete(QOIDecoder *self);

/**
 * Decodes the given QOI file contents.
 * Returns <code>true</code> if decoded successfully.
 * @param self This <code>QOIDecoder</code>.
 * @param encoded QOI file contents. Only the first <code>encodedSize</code> bytes are accessed.
 * @param encodedSize QOI file length.
 */
bool QOIDecoder_Decode(QOIDecoder *self, uint8_t const *encoded, int encodedSize);

/**
 * Returns the width of the decoded image in pixels.
 * @param self This <code>QOIDecoder</code>.
 */
int QOIDecoder_GetWidth(const QOIDecoder *self);

/**
 * Returns the height of the decoded image in pixels.
 * @param self This <code>QOIDecoder</code>.
 */
int QOIDecoder_GetHeight(const QOIDecoder *self);

/**
 * Returns the pixels of the decoded image, top-down, left-to-right.
 * Each pixel is a 32-bit integer 0xAARRGGBB.
 * @param self This <code>QOIDecoder</code>.
 */
int const *QOIDecoder_GetPixels(const QOIDecoder *self);

/**
 * Returns the information about the alpha channel from the file header.
 * @param self This <code>QOIDecoder</code>.
 */
bool QOIDecoder_HasAlpha(const QOIDecoder *self);

/**
 * Returns the color space information from the file header.
 * <code>false</code> = sRGB with linear alpha channel.
 * <code>true</code> = all channels linear.
 * @param self This <code>QOIDecoder</code>.
 */
bool QOIDecoder_IsLinearColorspace(const QOIDecoder *self);

#ifdef __cplusplus
}
#endif
