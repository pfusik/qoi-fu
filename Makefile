CFLAGS = -O2 -Wall
GIMP_LDFLAGS = `gimptool-2.0 --libs`
ifeq ($(OS),Windows_NT)
EXEEXT = .exe
GIMP_LDFLAGS += -Wl,-subsystem,windows
XNVIEW_DIR = C:/Program Files/XnViewMP
SUDO = elevate
else
XNVIEW_DIR = /opt/XnView
endif
CSC = "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/Roslyn/csc.exe" -nologo
PAINT_NET_DIR = C:/Program Files/paint.net
DOTNET_REF_DIR = C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Ref/6.0.0/ref/net6.0
TRANSPILED = $(addprefix transpiled/QOI., c cpp cs js py swift) transpiled/QOIDecoder.java

all: png2qoi Xqoi.usr $(TRANSPILED)

png2qoi: png2qoi.c QOI-stdio.c QOI-stdio.h transpiled/QOI.c
	$(CC) $(CFLAGS) -I transpiled -o $@ png2qoi.c QOI-stdio.c transpiled/QOI.c -lpng

file-qoi$(EXEEXT): file-qoi.c QOI-stdio.c QOI-stdio.h transpiled/QOI.c
	$(CC) $(CFLAGS) -I transpiled `gimptool-2.0 --cflags` -o $@ file-qoi.c QOI-stdio.c transpiled/QOI.c $(GIMP_LDFLAGS)

Xqoi.usr: Xqoi.c QOI-stdio.c QOI-stdio.h transpiled/QOI.c
	$(CC) $(CFLAGS) -I transpiled -o $@ Xqoi.c QOI-stdio.c transpiled/QOI.c -shared

QOIPaintDotNet.dll: QOIPaintDotNet.cs transpiled/QOI.cs
	$(CSC) -o+ -out:$@ -t:library $^ -nostdlib -r:"$(PAINT_NET_DIR)/PaintDotNet.Base.dll" -r:"$(PAINT_NET_DIR)/PaintDotNet.Core.dll" -r:"$(PAINT_NET_DIR)/PaintDotNet.Data.dll" -r:"$(DOTNET_REF_DIR)/System.Runtime.dll"

install-gimp: file-qoi$(EXEEXT)
ifeq ($(OS),Windows_NT)
	# gimptool-2.0 broken on mingw64
	install -D $< `gimptool-2.0 --gimpplugindir`/plug-ins/file-qoi.exe
else
	gimptool-2.0 --install-admin-bin $<
endif

install-xnview: Xqoi.usr
	$(SUDO) cp $< "$(XNVIEW_DIR)/Plugins/Xqoi.usr"

install-paint.net: QOIPaintDotNet.dll
	$(SUDO) cp $< "$(PAINT_NET_DIR)/FileTypes/QOIPaintDotNet.dll"

$(TRANSPILED): QOI.ci
	mkdir -p $(@D) && cito -o $@ $^

clean:
	$(RM) png2qoi file-qoi$(EXEEXT) Xqoi.usr QOIPaintDotNet.dll $(TRANSPILED) transpiled/QOI.h transpiled/QOI.hpp transpiled/QOIColorspace.java transpiled/QOIEncoder.java

.PHONY: all install-gimp install-xnview install-paint.net clean
