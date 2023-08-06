Name: qoi-fu
Version: 2.0.0
Release: 1
Summary: Quite OK Image (QOI) encoder/decoder
License: MIT
Source: https://github.com/pfusik/qoi-fu/archive/qoi-fu-%{version}/qoi-fu-%{version}.tar.gz
URL: https://github.com/pfusik/qoi-fu
BuildRequires: gcc

%description
Quite OK Image (QOI) encoder/decoder.

%package 2png
Summary: Command-line converter between QOI and PNG formats
BuildRequires: libpng-devel

%description 2png
Provides "png2qoi" command-line converter between the Quite OK Image (QOI)
and PNG formats.

%package gdk-pixbuf
Summary: GdkPixbuf loader for QOI image files
BuildRequires: gdk-pixbuf2-devel

%description gdk-pixbuf
GdkPixbuf loader for the Quite OK Image (QOI) format.

%package gimp
Summary: QOI plugin for GIMP
Requires: gimp
BuildRequires: gimp-devel

%description gimp
GIMP plugin for loading and exporting the Quite OK Image (QOI) format.

%prep
%setup -q

%build
make CFLAGS="%{build_cflags}" png2qoi libpixbufloader-qoi.so file-qoi

%install
make PREFIX=%{buildroot}%{_prefix} GDK_PIXBUF_LOADERS_DIR=%{buildroot}%{_libdir}/gdk-pixbuf-2.0/2.10.0/loaders libdir=%{buildroot}%{_libdir} BUILDING_PACKAGE=1 install-png2qoi install-gdk-pixbuf install-gimp

%files 2png
%{_bindir}/png2qoi

%files gdk-pixbuf
%{_libdir}/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-qoi.so
%{_datadir}/mime/packages/qoi-mime.xml
%{_datadir}/thumbnailers/qoi.thumbnailer

%files gimp
%{_libdir}/gimp/2.0/plug-ins/file-qoi/file-qoi

%changelog
* Mon Sep 12 2022 Piotr Fusik <fox@scene.pl>
- 2.0.0-1

* Wed Aug 3 2022 Piotr Fusik <fox@scene.pl>
- 1.1.2-1

* Mon Apr 18 2022 Piotr Fusik <fox@scene.pl>
- 1.1.1-1
- Initial packaging
