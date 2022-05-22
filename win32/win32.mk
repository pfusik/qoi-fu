VERSION = 1.1.1

IMAGINE_DIR = $(LOCALAPPDATA)/Imagine
CSC = "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/Roslyn/csc.exe" -nologo
PAINT_NET_DIR = C:/Program Files/paint.net
DOTNET_REF_DIR = `echo C:/Program\ Files/dotnet/packs/Microsoft.NETCore.App.Ref/6.0.*/ref/net6.0`
DO_SIGN = signtool sign -d "Quite OK Image plugins $(VERSION)" -n "Open Source Developer, Piotr Fusik" -tr http://time.certum.pl -fd sha256 -td sha256 $^ && touch $@

win32/QOI.plg64: win32/qoiimagine.c transpiled/QOI.c
	$(CC) $(CFLAGS) -I . -I transpiled -o $@ win32/qoiimagine.c transpiled/QOI.c -shared

win32/QOIPaintDotNet.dll: win32/QOIPaintDotNet.cs transpiled/QOI.cs
	$(CSC) -o+ -out:$@ -t:library $^ -nostdlib -r:"$(PAINT_NET_DIR)/PaintDotNet.Base.dll" -r:"$(PAINT_NET_DIR)/PaintDotNet.Core.dll" -r:"$(PAINT_NET_DIR)/PaintDotNet.Data.dll" -r:"$(DOTNET_REF_DIR)/System.Runtime.dll"

install-imagine: win32/QOI.plg64
	$(SUDO) cp $< "$(IMAGINE_DIR)/Plugin/QOI.plg64"

install-paint.net: win32/QOIPaintDotNet.dll
	$(SUDO) cp $< "$(PAINT_NET_DIR)/FileTypes/QOIPaintDotNet.dll"

../qoi-ci-$(VERSION)-win64.msi: win32/setup/qoi-ci.wixobj win32/setup/qoi.ico win32/setup/license.rtf win32/setup/dialog.jpg win32/setup/banner.jpg \
	file-qoi.exe win32/QOI.plg64 win32/QOIPaintDotNet.dll Xqoi.usr win32/signed
	light -nologo -o $@ -spdb -ext WixUIExtension -sice:ICE69 -sice:ICE80 $<

win32/setup/qoi-ci.wixobj: win32/setup/qoi-ci.wxs
	candle -nologo -o $@ -dVERSION=$(VERSION) -arch x64 $<

win32/setup/signed: ../qoi-ci-$(VERSION)-win64.msi
	$(DO_SIGN)

win32/signed: file-qoi.exe win32/QOI.plg64 win32/QOIPaintDotNet.dll Xqoi.usr
	$(DO_SIGN)

deb64:
	/usr/bin/tar czf ../qoi-ci-$(VERSION).tar.gz --numeric-owner --owner=0 --group=0 --mode=644 --transform=s,,qoi-ci-$(VERSION)/, `git ls-files`
	scp ../qoi-ci-$(VERSION).tar.gz vm:qoi-ci-$(VERSION).tar
	ssh vm 'rm -rf qoi-ci-$(VERSION) && tar xf qoi-ci-$(VERSION).tar && make -C qoi-ci-$(VERSION) deb'
	scp vm:qoi-ci-gimp_$(VERSION)-1_amd64.deb ..
	scp vm:qoi-ci-xnview_$(VERSION)-1_amd64.deb ..

rpm64:
	/usr/bin/tar czf ../qoi-ci-$(VERSION).tar.gz --numeric-owner --owner=0 --group=0 --mode=644 --transform=s,,qoi-ci-$(VERSION)/, `git ls-files`
	scp ../qoi-ci-$(VERSION).tar.gz vm:.
	ssh vm 'rpmbuild -tb qoi-ci-$(VERSION).tar.gz'
	scp vm:rpmbuild/RPMS/x86_64/qoi-ci-gimp-$(VERSION)-1.x86_64.rpm ..

CLEAN += win32/QOI.plg64 win32/QOIPaintDotNet.dll win32/setup/qoi-ci.wixobj win32/setup/signed win32/signed

.PHONY: install-imagine install-paint.net deb64 rpm64
