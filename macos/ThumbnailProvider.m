// ThumbnailProvider.m - QOI plugin for macOS Quick Look
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

#import "ThumbnailProvider.h"
#import "qoiToCGImage.h"

@implementation ThumbnailProvider

- (void)provideThumbnailForFileRequest:(QLFileThumbnailRequest *)request
		completionHandler:(void (^)(QLThumbnailReply * _Nullable reply, NSError * _Nullable error))handler
{
	NSError *error = nil;
	CGImageRef image = qoiToCGImage(request.fileURL, &error);
	if (!image) {
		handler(nil, error);
		return;
	}
	id imageOwner = CFBridgingRelease(image);
	CGSize size = request.maximumSize;
	size_t imageWidth = CGImageGetWidth(image);
	size_t imageHeight = CGImageGetHeight(image);
	if (imageWidth * size.height < imageHeight * size.width)
		size.width = imageWidth * size.height / imageHeight;
	else
		size.height = imageHeight * size.width / imageWidth;
	CGFloat width = size.width * request.scale;
	CGFloat height = size.height * request.scale;
	handler([QLThumbnailReply replyWithContextSize:size
		drawingBlock:^BOOL(CGContextRef context) {
			CGContextDrawImage(context, CGRectMake(0, 0, width, height), (__bridge CGImageRef)imageOwner);
			return YES;
		}], nil);
}

@end
