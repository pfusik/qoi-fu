VERSION = 0.3.0

../qoi-ci-$(VERSION)-win64.msi: win32/qoi-ci.wixobj win32/license.rtf file-qoi.exe QOIPaintDotNet.dll Xqoi.usr
	light -nologo -o $@ -spdb -ext WixUIExtension -sice:ICE69 -sice:ICE80 $<

win32/qoi-ci.wixobj: win32/qoi-ci.wxs
	candle -nologo -o $@ -dVERSION=$(VERSION) -arch x64 $<
