macos/bin/file-qoi: file-qoi macos/gimp-entitlements.xml
	mkdir -p $(@D) && cp $< $@
ifdef PORK_CODESIGNING_IDENTITY
	codesign --options runtime -f -s $(PORK_CODESIGNING_IDENTITY) --entitlements macos/gimp-entitlements.xml $@
endif

macos/bin/GIMP:
	mkdir -p $(@D) && ln -s /Applications/GIMP-2.10.app/Contents/Resources/lib/gimp/2.0/plug-ins $@

macos/qoi-ci-$(VERSION)-macos.dmg: macos/bin/file-qoi macos/bin/GIMP
	hdiutil create -volname qoi-ci-$(VERSION)-macos -srcfolder macos/bin -format UDBZ -fs HFS+ -imagekey bzip2-level=3 -ov $@
ifdef PORK_NOTARIZING_CREDENTIALS
	xcrun altool --notarize-app --primary-bundle-id qoi-ci $(PORK_NOTARIZING_CREDENTIALS) --file $@ \
		| perl -pe 's/^RequestUUID =/xcrun altool $$ENV{PORK_NOTARIZING_CREDENTIALS} --notarization-info/ or next; $$c = $$_; until (/Status: success/) { sleep 20; $$_ = `$$c`; print; } last;'
endif

CLEAN += macos/bin/file-qoi macos/bin/GIMP
