VERSION = 0.3.0

DO_SIGN = signtool sign -d "Quite OK Image plugins $(VERSION)" -n "Open Source Developer, Piotr Fusik" -tr http://time.certum.pl -fd sha256 -td sha256 $^ && touch $@

../qoi-ci-$(VERSION)-win64.msi: win32/qoi-ci.wixobj win32/license.rtf file-qoi.exe QOIPaintDotNet.dll Xqoi.usr win32/signed
	light -nologo -o $@ -spdb -ext WixUIExtension -sice:ICE69 -sice:ICE80 $<

win32/qoi-ci.wixobj: win32/qoi-ci.wxs
	candle -nologo -o $@ -dVERSION=$(VERSION) -arch x64 $<

win32/signed-msi: ../qoi-ci-$(VERSION)-win64.msi
	$(DO_SIGN)

win32/signed: file-qoi.exe QOIPaintDotNet.dll Xqoi.usr
	$(DO_SIGN)
