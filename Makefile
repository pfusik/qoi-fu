VERSION = 2.0.0

PREFIX = /usr/local
CFLAGS = -O2 -Wall
GDK_PIXBUF_LOADERS_DIR = `pkg-config --variable=gdk_pixbuf_moduledir gdk-pixbuf-2.0`
GIMP_LDFLAGS = `gimptool-2.0 --libs`
GIMP_MACOS_LIBDIR = /Applications/GIMP-2.10.app/Contents/Resources/lib
ifeq ($(OS),Windows_NT)
EXEEXT = .exe
GIMP_LDFLAGS += -Wl,-subsystem,windows
XNVIEW_DIR = C:/Program Files/XnViewMP
SUDO = elevate
else ifneq ($(wildcard $(GIMP_MACOS_LIBDIR)),)
GIMP_LDFLAGS = -mmacosx-version-min=10.9 -rpath @executable_path/../../../ -L $(GIMP_MACOS_LIBDIR) -lgimpui-2.0.0 -lgimp-2.0.0 -lgegl-0.4.0 -lbabl-0.1.0 -lgobject-2.0.0
else
XNVIEW_DIR = /opt/XnView
endif
TRANSPILED = $(addprefix transpiled/QOI., c cpp cs js py swift ts) transpiled/QOIDecoder.java

all: png2qoi$(EXEEXT) Xqoi.usr $(TRANSPILED)

png2qoi$(EXEEXT): png2qoi.c QOI-stdio.c QOI-stdio.h transpiled/QOI.c
	$(CC) $(CFLAGS) -I transpiled -o $@ png2qoi.c QOI-stdio.c transpiled/QOI.c $(if $(EXEEXT), -static) -lpng -lz

libpixbufloader-qoi.so: io-qoi.c QOI-stdio.c QOI-stdio.h transpiled/QOI.c
	$(CC) $(CFLAGS) `pkg-config --cflags gdk-pixbuf-2.0` -I transpiled -o $@ io-qoi.c QOI-stdio.c transpiled/QOI.c -fvisibility=hidden -shared -fPIC -lgdk_pixbuf-2.0 -lglib-2.0

file-qoi$(EXEEXT): file-qoi.c QOI-stdio.c QOI-stdio.h transpiled/QOI.c
	$(CC) $(CFLAGS) -I transpiled `gimptool-2.0 --cflags` -o $@ file-qoi.c QOI-stdio.c transpiled/QOI.c $(GIMP_LDFLAGS)

Xqoi.usr: Xqoi.c QOI-stdio.c QOI-stdio.h transpiled/QOI.c
	$(CC) $(CFLAGS) -I transpiled -o $@ Xqoi.c QOI-stdio.c transpiled/QOI.c -shared -fPIC

install-png2qoi: png2qoi$(EXEEXT)
	install -D $< $(PREFIX)/bin/png2qoi$(EXEEXT)

install-gdk-pixbuf: libpixbufloader-qoi.so qoi.thumbnailer install-mime
	install -D -m 644 $< $(GDK_PIXBUF_LOADERS_DIR)/libpixbufloader-qoi.so
	install -D -m 644 qoi.thumbnailer $(PREFIX)/share/thumbnailers/qoi.thumbnailer

install-mime: qoi-mime.xml
	install -D -m 644 $< $(PREFIX)/share/mime/packages/qoi-mime.xml
ifndef BUILDING_PACKAGE
	update-mime-database $(PREFIX)/share/mime
endif

install-gimp: file-qoi$(EXEEXT)
ifeq ($(OS),Windows_NT)
	# gimptool-2.0 --install-admin-bin broken on mingw64
	install -D $< `gimptool-2.0 --gimpplugindir`/plug-ins/file-qoi.exe
else ifdef BUILDING_PACKAGE
	install -D $< $(libdir)/gimp/2.0/plug-ins/file-qoi/file-qoi
else ifneq ($(wildcard $(GIMP_MACOS_LIBDIR)),)
	cp $< $(GIMP_MACOS_LIBDIR)/gimp/2.0/plug-ins/file-qoi
else
	gimptool-2.0 --install-admin-bin $<
endif

install-xnview: Xqoi.usr
	$(SUDO) install -D -m 644 $< "$(XNVIEW_DIR)/Plugins/Xqoi.usr"

$(TRANSPILED): QOI.fu
	mkdir -p $(@D) && fut -o $@ $^

CLEAN = png2qoi$(EXEEXT) libpixbufloader-qoi.so file-qoi$(EXEEXT) Xqoi.usr $(TRANSPILED) transpiled/QOI.h transpiled/QOI.hpp transpiled/QOIEncoder.java
clean:
	$(RM) $(CLEAN)

deb:
	debuild -b -us -uc

.PHONY: all install-png2qoi install-gdk-pixbuf install-mime install-gimp install-xnview clean deb

include macos/macos.mk
include win32/win32.mk
