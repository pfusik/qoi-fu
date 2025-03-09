macos/bin/png2qoi: png2qoi.c QOI-stdio.c QOI-stdio.h transpiled/QOI.c
	mkdir -p $(@D) && cc -O2 -Wall -I transpiled -mmacosx-version-min=10.6 -arch x86_64 -arch arm64 -o $@ png2qoi.c QOI-stdio.c transpiled/QOI.c /usr/local/lib/libpng.a -lz
ifdef FOX_CODESIGNING_IDENTITY
	codesign --options runtime -f -s $(FOX_CODESIGNING_IDENTITY) $@
endif

macos/bin/bin:
	mkdir -p $(@D) && ln -s /usr/local/bin $@

macos/bin/QOI.qlgenerator/Contents/_CodeSignature/CodeResources: macos/bin/QOI.qlgenerator/Contents/MacOS/qlqoi macos/bin/QOI.qlgenerator/Contents/Info.plist macos/ql-entitlements.xml
ifdef FOX_CODESIGNING_IDENTITY
	codesign --options runtime -f -s $(FOX_CODESIGNING_IDENTITY) --entitlements macos/ql-entitlements.xml macos/bin/QOI.qlgenerator
else
	rm -rf $(@D)
endif

macos/bin/QOI.qlgenerator/Contents/MacOS/qlqoi: macos/qlqoi.c transpiled/QOI.c
	mkdir -p $(@D) && $(CC) $(CFLAGS) -I transpiled -mmacosx-version-min=10.6 -arch x86_64 -arch arm64 -o $@ $^ -dynamiclib -framework QuickLook -framework ApplicationServices -framework CoreFoundation && strip -x $@

macos/bin/QOI.qlgenerator/Contents/Info.plist: macos/Info.plist
	mkdir -p $(@D) && cp $< $@

macos/bin/QuickLook:
	mkdir -p $(@D) && ln -s /Library/QuickLook $@

macos/qoi-fu-$(VERSION)-macos.dmg: macos/bin/png2qoi macos/bin/bin macos/bin/QOI.qlgenerator/Contents/_CodeSignature/CodeResources macos/bin/QuickLook
	hdiutil create -volname qoi-fu-$(VERSION)-macos -srcfolder macos/bin -format UDBZ -fs HFS+ -imagekey bzip2-level=3 -ov $@
	/Applications/Xcode.app/Contents/Developer/usr/bin/notarytool submit --wait --keychain-profile foxnotary $@

CLEAN += macos/bin/png2qoi macos/bin/bin macos/bin/QOI.qlgenerator/Contents/MacOS/qlqoi macos/bin/QOI.qlgenerator/Contents/Info.plist macos/bin/QuickLook
