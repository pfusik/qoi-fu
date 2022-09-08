// qlqoi.c - QOI plugin for macOS QuickLook
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

#include <libkern/OSAtomic.h>
#include <CoreFoundation/CFPlugInCOM.h>
#include <QuickLook/QuickLook.h>

#include "QOI.h"

static CGImageRef CreateImage(CFURLRef url)
{
	CFDataRef data;
	SInt32 errorCode;
	if (!CFURLCreateDataAndPropertiesFromResource(NULL, url, &data, NULL, NULL, &errorCode))
		return NULL;

	QOIDecoder *qoi = QOIDecoder_New();
	if (qoi == NULL) {
		CFRelease(data);
		return NULL;
	}
	if (!QOIDecoder_Decode(qoi, CFDataGetBytePtr(data), CFDataGetLength(data))) {
		QOIDecoder_Delete(qoi);
		CFRelease(data);
		return NULL;
	}
	CFRelease(data);

	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	if (colorSpace == NULL) {
		QOIDecoder_Delete(qoi);
		return NULL;
	}
	int width = QOIDecoder_GetWidth(qoi);
	int height = QOIDecoder_GetHeight(qoi);
	CGContextRef context = CGBitmapContextCreate((void *) QOIDecoder_GetPixels(qoi), width, height, 8, width << 2, colorSpace, kCGBitmapByteOrder32Host | kCGImageAlphaPremultipliedFirst);
	CFRelease(colorSpace);
	if (context == NULL) {
		QOIDecoder_Delete(qoi);
		return NULL;
	}
	CGImageRef image = CGBitmapContextCreateImage(context);
	CFRelease(context);
	QOIDecoder_Delete(qoi);
	return image;
}

typedef struct
{
	const QLGeneratorInterfaceStruct *vtbl;
	int refCount;
	CFUUIDRef factoryID;
} Plugin;

static ULONG AddRef(void *thisInstance)
{
	Plugin *plugin = (Plugin *) thisInstance;
	return OSAtomicIncrement32(&plugin->refCount);
}

static HRESULT QueryInterface(void *thisInstance, REFIID iid, LPVOID *ppv)
{
	CFUUIDRef interfaceID = CFUUIDCreateFromUUIDBytes(kCFAllocatorDefault, iid);
	if (CFEqual(interfaceID, kQLGeneratorCallbacksInterfaceID)) {
		CFRelease(interfaceID);
		AddRef(thisInstance);
		*ppv = thisInstance;
		return S_OK;
	}
	CFRelease(interfaceID);
	*ppv = NULL;
	return E_NOINTERFACE;
}

static void Dealloc(Plugin *plugin)
{
	CFUUIDRef factoryID = plugin->factoryID;
	CFPlugInRemoveInstanceForFactory(factoryID);
	CFRelease(factoryID);
	free(plugin);
}

static ULONG Release(void *thisInstance)
{
	Plugin *plugin = (Plugin *) thisInstance;
	ULONG r = OSAtomicDecrement32(&plugin->refCount);
	if (r == 0)
		Dealloc(plugin);
	return r;
}

static OSStatus GenerateThumbnailForURL(void *thisInstance, QLThumbnailRequestRef thumbnail, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize)
{
	CGImageRef image = CreateImage(url);
	if (image != NULL) {
		QLThumbnailRequestSetImage(thumbnail, image, NULL);
		CFRelease(image);
	}
	return noErr;
}

static void CancelThumbnailGeneration(void* thisInstance, QLThumbnailRequestRef thumbnail)
{
}

static OSStatus GeneratePreviewForURL(void *thisInstance, QLPreviewRequestRef preview, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options)
{
	CGImageRef image = CreateImage(url);
	if (image == NULL)
		return noErr;

	size_t width = CGImageGetWidth(image);
	size_t height = CGImageGetHeight(image);
	CGContextRef context = QLPreviewRequestCreateContext(preview, CGSizeMake(width, height), true, NULL);
	if (context == NULL) {
		CGImageRelease(image);
		return noErr;
	}
	CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
	CGImageRelease(image);
	QLPreviewRequestFlushContext(preview, context);
	CFRelease(context);
	return noErr;
}

static void CancelPreviewGeneration(void *thisInstance, QLPreviewRequestRef preview)
{
}

void *QuickLookGeneratorPluginFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
	if (CFEqual(typeID, kQLGeneratorTypeID)) {
		static const QLGeneratorInterfaceStruct pluginVtbl = {
			NULL,
			QueryInterface,
			AddRef,
			Release,
			GenerateThumbnailForURL,
			CancelThumbnailGeneration,
			GeneratePreviewForURL,
			CancelPreviewGeneration
		};
		Plugin *plugin = (Plugin *) malloc(sizeof(Plugin));
		plugin->vtbl = &pluginVtbl;
		plugin->refCount = 1;
		plugin->factoryID = CFUUIDCreateFromString(kCFAllocatorDefault, CFSTR("EBD90849-B1FC-4285-850B-76BD57E5C97F"));
		CFPlugInAddInstanceForFactory(plugin->factoryID);
		return plugin;
	}
	return NULL;
}
