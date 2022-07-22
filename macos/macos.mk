macos/bin/file-qoi: file-qoi macos/gimp-entitlements.xml
	mkdir -p $(@D) && cp $< $@
ifdef PORK_CODESIGNING_IDENTITY
	codesign --options runtime -f -s $(PORK_CODESIGNING_IDENTITY) --entitlements macos/gimp-entitlements.xml $@
endif

CLEAN += macos/bin/file-qoi
