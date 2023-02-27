CXXFLAGS = -O2 -Wall
IMAGINE_DIR = $(LOCALAPPDATA)/Imagine
IMAGINE32_DIR = $(LOCALAPPDATA)/Imagine32
XNVIEW32_DIR = C:/Program Files (x86)/XnView
CC32 = i686-w64-mingw32-gcc
CXX32 = i686-w64-mingw32-g++
CSC = "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/Roslyn/csc.exe" -nologo
PAINT_NET_DIR = C:/Program Files/paint.net
DOTNET_REF_DIR = `echo C:/Program\ Files/dotnet/packs/Microsoft.NETCore.App.Ref/7.0.*/ref/net7.0`
DO_SIGN = signtool sign -d "Quite OK Image plugins $(VERSION)" -n "Open Source Developer, Piotr Fusik" -tr http://time.certum.pl -fd sha256 -td sha256 $^ && touch $@

win32/wicqoi64.dll: win32/wicqoi.cpp win32/QOI.o
	$(CXX) $(CXXFLAGS) -I transpiled -o $@ $^ -shared -lwindowscodecs -lole32

win32/wicqoi32.dll: win32/wicqoi.cpp win32/QOI32.o
	$(CXX32) $(CXXFLAGS) -I transpiled -o $@ $^ -shared -Wl,--kill-at -lwindowscodecs -lole32

win32/QOI.o: transpiled/QOI.c
	$(CC) $(CFLAGS) -c -o $@ $^

win32/QOI32.o: transpiled/QOI.c
	$(CC32) $(CFLAGS) -c -o $@ $^

win32/QOI.plg64: win32/qoiimagine.c transpiled/QOI.c
	$(CC) $(CFLAGS) -I . -I transpiled -o $@ $^ -shared

win32/QOI.plg: win32/qoiimagine.c transpiled/QOI.c
	$(CC32) $(CFLAGS) -I . -I transpiled -o $@ $^ -shared -Wl,--kill-at

win32/Xqoi32.usr: Xqoi.c QOI-stdio.c QOI-stdio.h transpiled/QOI.c
	$(CC32) $(CFLAGS) -I transpiled -o $@ $^ -shared -Wl,--kill-at

win32/QOIPaintDotNet.dll: win32/QOIPaintDotNet.cs transpiled/QOI.cs
	$(CSC) -o+ -out:$@ -t:library $^ -nostdlib -r:"$(PAINT_NET_DIR)/PaintDotNet.ComponentModel.dll" -r:"$(PAINT_NET_DIR)/PaintDotNet.Core.dll" -r:"$(PAINT_NET_DIR)/PaintDotNet.Data.dll" -r:"$(PAINT_NET_DIR)/PaintDotNet.Primitives.dll" -r:"$(DOTNET_REF_DIR)/System.Runtime.dll"

install-imagine: win32/QOI.plg64
	cp $< "$(IMAGINE_DIR)/Plugin/QOI.plg64"

install-imagine32: win32/QOI.plg
	cp $< "$(IMAGINE32_DIR)/Plugin/QOI.plg"

install-xnview32: win32/Xqoi32.usr
	$(SUDO) cp $< "$(XNVIEW32_DIR)/PlugIns/Xqoi32.usr"

install-paint.net: win32/QOIPaintDotNet.dll
	$(SUDO) cp $< "$(PAINT_NET_DIR)/FileTypes/QOIPaintDotNet.dll"

../qoi-ci-$(VERSION)-win64.msi: win32/setup/qoi-ci.wixobj win32/setup/qoi.ico win32/setup/license.rtf win32/setup/dialog.jpg win32/setup/banner.jpg \
	png2qoi.exe file-qoi.exe win32/wicqoi64.dll win32/wicqoi32.dll win32/QOI.plg64 win32/QOI.plg win32/Xqoi32.usr win32/QOIPaintDotNet.dll Xqoi.usr win32/signed
	light -nologo -o $@ -spdb -ext WixUIExtension -sice:ICE69 -sice:ICE80 $<

win32/setup/qoi-ci.wixobj: win32/setup/qoi-ci.wxs
	candle -nologo -o $@ -dVERSION=$(VERSION) -arch x64 $<

win32/setup/signed: ../qoi-ci-$(VERSION)-win64.msi
	$(DO_SIGN)

win32/signed: png2qoi.exe win32/wicqoi64.dll win32/wicqoi32.dll file-qoi.exe win32/QOI.plg64 win32/QOI.plg win32/Xqoi32.usr win32/QOIPaintDotNet.dll Xqoi.usr
	$(DO_SIGN)

deb64:
	/usr/bin/tar czf ../qoi-ci-$(VERSION).tar.gz --numeric-owner --owner=0 --group=0 --mode=644 --transform=s,,qoi-ci-$(VERSION)/, `git ls-files`
	scp ../qoi-ci-$(VERSION).tar.gz vm:qoi-ci-$(VERSION).tar
	ssh vm 'rm -rf qoi-ci-$(VERSION) && tar xf qoi-ci-$(VERSION).tar && make -C qoi-ci-$(VERSION) deb'
	scp vm:qoi-ci-2png_$(VERSION)-1_amd64.deb ..
	scp vm:qoi-ci-gdk-pixbuf_$(VERSION)-1_amd64.deb ..
	scp vm:qoi-ci-gimp_$(VERSION)-1_amd64.deb ..
	scp vm:qoi-ci-xnview_$(VERSION)-1_amd64.deb ..

rpm64:
	/usr/bin/tar czf ../qoi-ci-$(VERSION).tar.gz --numeric-owner --owner=0 --group=0 --mode=644 --transform=s,,qoi-ci-$(VERSION)/, `git ls-files`
	scp ../qoi-ci-$(VERSION).tar.gz vm:.
	ssh vm 'rpmbuild -tb qoi-ci-$(VERSION).tar.gz'
	scp vm:rpmbuild/RPMS/x86_64/qoi-ci-2png-$(VERSION)-1.x86_64.rpm ..
	scp vm:rpmbuild/RPMS/x86_64/qoi-ci-gdk-pixbuf-$(VERSION)-1.x86_64.rpm ..
	scp vm:rpmbuild/RPMS/x86_64/qoi-ci-gimp-$(VERSION)-1.x86_64.rpm ..

mac:
	/usr/bin/tar czf ../qoi-ci-$(VERSION).tar.gz --numeric-owner --owner=0 --group=0 --mode=644 --transform=s,,qoi-ci-$(VERSION)/, `git ls-files`
	scp ../qoi-ci-$(VERSION).tar.gz mac:.
	ssh mac 'security unlock-keychain ~/Library/Keychains/login.keychain && rm -rf qoi-ci-$(VERSION) && tar xf qoi-ci-$(VERSION).tar.gz && PATH=/usr/local/bin:$$PATH make -C qoi-ci-$(VERSION) macos/qoi-ci-$(VERSION)-macos.dmg'
	scp mac:qoi-ci-$(VERSION)/macos/qoi-ci-$(VERSION)-macos.dmg ..

CLEAN += win32/wicqoi64.dll win32/wicqoi32.dll win32/QOI.o win32/QOI32.o win32/QOI.plg64 win32/QOI.plg win32/Xqoi32.usr win32/QOIPaintDotNet.dll win32/setup/qoi-ci.wixobj win32/setup/signed win32/signed

.PHONY: install-imagine install-imagine32 install-xnview32 install-paint.net deb64 rpm64 mac
