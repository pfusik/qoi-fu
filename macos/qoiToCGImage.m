// qoiToCGImage.m - QOI plugin for macOS Quick Look
//
// Copyright (C) 2026 Piotr Fusik
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

#import "qoiToCGImage.h"
#import "QOI.h"

static void releaseQOI(void *info, const void *data, size_t size)
{
	QOIDecoder_Delete(info);
}

CGImageRef qoiToCGImage(NSURL *url, NSError **error)
{
	NSData *data = [NSData dataWithContentsOfURL:url options:0 error:error];
	if (!data)
		return NULL;
	QOIDecoder *qoi = QOIDecoder_New();
	if (!QOIDecoder_Decode(qoi, data.bytes, (int)data.length)) {
		QOIDecoder_Delete(qoi);
		*error = [NSError errorWithDomain:@"org.fusion-lang.qoi.ErrorDomain" code:1 userInfo:nil];
		return NULL;
	}
	size_t width = QOIDecoder_GetWidth(qoi);
	size_t height = QOIDecoder_GetHeight(qoi);
	CGDataProviderRef provider = CGDataProviderCreateWithData(qoi,
		QOIDecoder_GetPixels(qoi), width * height << 2, releaseQOI);
	if (!provider) {
		QOIDecoder_Delete(qoi);
		*error = [NSError errorWithDomain:@"org.fusion-lang.qoi.ErrorDomain" code:2 userInfo:nil];
		return NULL;
	}
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	CGImageRef image = CGImageCreate(width, height, 8, 32, width << 2, space,
		(CGBitmapInfo)kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host,
		provider, NULL, false, kCGRenderingIntentDefault);
	CGColorSpaceRelease(space);
	CGDataProviderRelease(provider);
	return image;
}
