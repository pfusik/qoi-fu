#import <Cocoa/Cocoa.h>

int main(int argc, char **argv)
{
	@autoreleasepool {
		[NSApplication sharedApplication];
		NSAlert *alert = [[NSAlert alloc] init];
		[alert setMessageText:@"QOI Quick Look plugin"];
		[alert setInformativeText:@"Plugin for the Quite OK image (QOI) format has been installed. Use Finder thumbnails and preview for QOI files. Thumbnails may not appear until you restart your computer."];
		[alert setAlertStyle:NSAlertStyleInformational];
		[alert addButtonWithTitle:@"OK"];
		[alert runModal];
		[NSApp terminate:nil];
	}
	return 0;
}
