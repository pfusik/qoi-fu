VERSION = 1.0.0

CSC = "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/Roslyn/csc.exe" -nologo
PAINT_NET_DIR = C:/Program Files/paint.net
DOTNET_REF_DIR = C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Ref/6.0.1/ref/net6.0
DO_SIGN = signtool sign -d "Quite OK Image plugins $(VERSION)" -n "Open Source Developer, Piotr Fusik" -tr http://time.certum.pl -fd sha256 -td sha256 $^ && touch $@

win32/QOIPaintDotNet.dll: win32/QOIPaintDotNet.cs transpiled/QOI.cs
	$(CSC) -o+ -out:$@ -t:library $^ -nostdlib -r:"$(PAINT_NET_DIR)/PaintDotNet.Base.dll" -r:"$(PAINT_NET_DIR)/PaintDotNet.Core.dll" -r:"$(PAINT_NET_DIR)/PaintDotNet.Data.dll" -r:"$(DOTNET_REF_DIR)/System.Runtime.dll"

install-paint.net: QOIPaintDotNet.dll
	$(SUDO) cp $< "$(PAINT_NET_DIR)/FileTypes/QOIPaintDotNet.dll"

../qoi-ci-$(VERSION)-win64.msi: win32/qoi-ci.wixobj win32/license.rtf file-qoi.exe win32/QOIPaintDotNet.dll Xqoi.usr win32/signed
	light -nologo -o $@ -spdb -ext WixUIExtension -sice:ICE69 -sice:ICE80 $<

win32/qoi-ci.wixobj: win32/qoi-ci.wxs
	candle -nologo -o $@ -dVERSION=$(VERSION) -arch x64 $<

win32/signed-msi: ../qoi-ci-$(VERSION)-win64.msi
	$(DO_SIGN)

win32/signed: file-qoi.exe win32/QOIPaintDotNet.dll Xqoi.usr
	$(DO_SIGN)

CLEAN += win32/QOIPaintDotNet.dll win32/signed-msi win32/signed

.PHONY: install-paint.net
