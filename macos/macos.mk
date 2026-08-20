OBJC = clang
OBJCFLAGS = -O2 -Wall -fobjc-arc -mmacos-version-min=12.0 -arch x86_64 -arch arm64
QLAPP = macos/bin/QOI-QuickLook.app

macos/bin/png2qoi: png2qoi.c QOI-stdio.c QOI-stdio.h transpiled/QOI.c
	mkdir -p $(@D) && clang -o $@ -O2 -Wall -mmacosx-version-min=10.6 -arch x86_64 -arch arm64 -I transpiled png2qoi.c QOI-stdio.c transpiled/QOI.c /usr/local/lib/libpng.a -lz
ifdef FOX_CODESIGNING_IDENTITY
	codesign --options runtime -f -s $(FOX_CODESIGNING_IDENTITY) $@
endif

macos/bin/bin:
	mkdir -p $(@D) && ln -s /usr/local/bin $@

$(QLAPP)/Contents/_CodeSignature/CodeResources: $(QLAPP)/Contents/MacOS/QOIMessage $(QLAPP)/Contents/Info.plist $(QLAPP)/Contents/Resources/AppIcon.icns \
	$(QLAPP)/Contents/PlugIns/QOIPreview.appex/Contents/MacOS/QOIPreview $(QLAPP)/Contents/PlugIns/QOIPreview.appex/Contents/Info.plist \
	$(QLAPP)/Contents/PlugIns/QOIThumbnail.appex/Contents/MacOS/QOIThumbnail $(QLAPP)/Contents/PlugIns/QOIThumbnail.appex/Contents/Info.plist \
	macos/ql-entitlements.xml
ifdef FOX_CODESIGNING_IDENTITY
	codesign --options runtime -f -s $(FOX_CODESIGNING_IDENTITY) --entitlements macos/ql-entitlements.xml $(QLAPP)/Contents/PlugIns/QOIPreview.appex $(QLAPP)/Contents/PlugIns/QOIThumbnail.appex $(QLAPP)
endif

$(QLAPP)/Contents/MacOS/QOIMessage: macos/message.m
	mkdir -p $(@D) && $(OBJC) -o $@ $(OBJCFLAGS) $^ -framework Cocoa && strip -x $@

$(QLAPP)/Contents/PlugIns/QOIPreview.appex/Contents/MacOS/QOIPreview: macos/PreviewProvider.m macos/qoiToCGImage.m transpiled/QOI.c
	mkdir -p $(@D) && $(OBJC) -o $@ $(OBJCFLAGS) -fapplication-extension -I transpiled $^ -e _NSExtensionMain -framework Quartz -framework CoreGraphics && strip -x $@

$(QLAPP)/Contents/PlugIns/QOIThumbnail.appex/Contents/MacOS/QOIThumbnail: macos/ThumbnailProvider.m macos/qoiToCGImage.m transpiled/QOI.c
	mkdir -p $(@D) && $(OBJC) -o $@ $(OBJCFLAGS) -fapplication-extension -I transpiled $^ -e _NSExtensionMain -framework QuickLookThumbnailing -framework CoreGraphics && strip -x $@

$(QLAPP)/Contents/Info.plist: macos/Info.plist
	mkdir -p $(@D) && cp $< $@

$(QLAPP)/Contents/PlugIns/QOIPreview.appex/Contents/Info.plist: macos/Preview.plist
	mkdir -p $(@D) && cp $< $@

$(QLAPP)/Contents/PlugIns/QOIThumbnail.appex/Contents/Info.plist: macos/Thumbnail.plist
	mkdir -p $(@D) && cp $< $@

$(QLAPP)/Contents/Resources/AppIcon.icns: macos/icon.png
	mkdir -p macos/icon.iconset $(@D)
	sips -z 16 16 $< -o macos/icon.iconset/icon_16x16.png
	sips -z 32 32 $< -o macos/icon.iconset/icon_16x16@2x.png
	sips -z 128 128 $< -o macos/icon.iconset/icon_128x128.png
	sips -z 256 256 $< -o macos/icon.iconset/icon_128x128@2x.png
	iconutil -c icns macos/icon.iconset -o $@

macos/bin/Applications:
	mkdir -p $(@D) && ln -s /Applications $@

macos/qoi-fu-$(VERSION)-macos.dmg: macos/bin/png2qoi macos/bin/bin $(QLAPP)/Contents/_CodeSignature/CodeResources macos/bin/Applications
	hdiutil create -volname qoi-fu-$(VERSION)-macos -srcfolder macos/bin -format UDBZ -fs HFS+ -imagekey bzip2-level=3 -ov $@
	/Applications/Xcode.app/Contents/Developer/usr/bin/notarytool submit --wait --keychain-profile foxnotary $@

CLEAN += macos/bin/png2qoi macos/bin/bin macos/bin/Applications
